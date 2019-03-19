#include "9cc.h"

static Node *stmt(Function *func);
static Node *assign(Function *func);
static Node *eq(Function *func);
static Node *add(Function *func);
static Node *mul(Function *func);
static Node *term(Function *func);

static Node *new_node(int ty, Node *lhs, Node *rhs){
    Node *node = malloc(sizeof(Node));
    node->ty = ty;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

static Node *new_node_num(int val){
    Node *node = malloc(sizeof(Node));
    node->ty = ND_NUM;
    node->val = val;
    return node;
}

static Node *new_node_ident(char *input){
    Node *node = malloc(sizeof(Node));
    node->ty = ND_IDENT;
    node->name = input;
    return node;
}

static Node *new_node_call(char *input){
    Node *node = malloc(sizeof(Node));
    node->ty = ND_CALL;
    node->name = input;
    return node;
}

static int consume(int ty){
    Token *token = (Token *)(tokens->data[pos]);
    if (token->ty != ty)
        return 0;
    pos++;
    return 1;
}

static void add_ident(Map *map, char *name){
    if (map_get(map, name) == NULL)
        map_put(map, name, (void *)(map->keys->len));
}

static Function *add_func(Vector *funcs, char *name){
    Function *func = malloc(sizeof(Function));
    func->name = name;
    func->idents = new_map();
    vec_push(funcs, (void *)func);
    return func;
}

Vector *program(){
    int i;
    Function *func;
    Vector *funcs = new_vector();
    while (((Token *)(tokens->data[pos]))->ty != TK_EOF){
        if (((Token *)(tokens->data[pos]))->ty != TK_IDENT){
            fprintf(stderr,"function name is needed");
            exit(1);
        } else {
            i = 0;
            func = add_func(funcs, ((Token *)(tokens->data[pos]))->input);
            pos++;
            consume('(');
            consume(')');
            consume('{');
            while (((Token *)(tokens->data[pos]))->ty != '}')
                func->code[i++] = stmt(func);
            func->code[i] = NULL;
            consume('}');
        }
    }
    return funcs;
}

static Node *stmt(Function *func){
    Token *token = (Token *)(tokens->data[pos]);
    Node *node = assign(func);
    if (!consume(';')){
        fprintf(stderr,"token without ';': %s", token->input);
        exit(1);
    }
    return node;
}        

static Node *assign(Function *func){
    Node *node = eq(func);
    
    for (;;){
        if (consume('='))
            node = new_node('=', node, assign(func));
        else
            return node;
    }
}

static Node *eq(Function *func){
    Node *node = add(func);

    for (;;){
        if (consume(TK_EQ))
            node = new_node(ND_EQ, node, add(func));
        else if (consume(TK_EQN))
            node = new_node(ND_EQN, node, add(func));
        else
            return node;
    }
}

static Node *add(Function *func){
    Node *node = mul(func);
    
    for (;;){
        if (consume('+'))
            node = new_node('+', node, mul(func));
        else if (consume('-'))
            node = new_node('-', node, mul(func));
        else
            return node;
    }
}

static Node *mul(Function *func){
    Node *node = term(func);

    for (;;){
        if (consume('*'))
            node = new_node('*', node, term(func));
        else if (consume('/'))
            node = new_node('/', node, term(func));
        else
            return node;
    }
}
    
static Node *term(Function *func){
    Token *token = (Token *)(tokens->data[pos]);

    if (consume('(')){
        Node *node = add(func);
        if (!consume(')')){
            fprintf(stderr,"'(' without ')': %s", 
                    token->input);
            exit(1);
        }
        return node;
    }

    if (consume(TK_NUM))
        return new_node_num(token->val);
    
    if (consume(TK_IDENT)){
        if (!consume('(')){
            add_ident(func->idents, token->input);
            return new_node_ident(token->input);
        }
        else {
            Node *node = new_node_call(token->input);
            node->args = new_vector();
            if (!consume(')')) { // get arguments
                do{
                     vec_push(node->args, (void *)assign(func));
                }while(consume(','));
                consume(')');
            }
            return node;
        }
    }

    fprintf(stderr,"found an unknown token: %s", token->input);
    exit(1);
}

static void gen_lval(Function *func, Node *node){
    if (node->ty != ND_IDENT){
        fprintf(stderr, "lhs is not a variable");
        exit(1);
    }

    int offset = (int)map_get(func->idents, node->name) * 8;
    printf("    mov rax, rbp\n");
    printf("    sub rax, %d\n", offset);
    printf("    push rax\n");
}

void gen(Function *func, Node *node){
    if (node->ty == ND_NUM){
        printf("    push %d\n", node->val);
        return;
    }

    if (node->ty == ND_IDENT){
        gen_lval(func, node);
        printf("    pop rax\n");
        printf("    mov rax, [rax]\n");
        printf("    push rax\n");
        return;
    }

    if (node->ty == ND_CALL){
        char * reg[] = {"rdi","rsi","rdx","rcx","r8","r9"};
        for (int i = node->args->len-1; i >= 0; i--){
            gen(func, (Node *)(node->args->data[i]));
            printf("    pop %s\n",reg[i]);
        }
        printf("    call %s\n", node->name);
        printf("    push rax\n");
        return;
    }

    if (node->ty == '='){
        gen_lval(func, node->lhs);
        gen(func, node->rhs);

        printf("    pop rdi\n");
        printf("    pop rax\n");
        printf("    mov [rax], rdi\n");
        printf("    push rdi\n");
        return;
    }
        
    gen(func, node->lhs);
    gen(func, node->rhs);

    printf("    pop rdi\n");
    printf("    pop rax\n");

    switch (node->ty){
        case ND_EQ:
            printf("    cmp rdi, rax\n");
            printf("    sete al\n");
            printf("    movzb rax, al\n");
            break;
        case ND_EQN:
            printf("    cmp rdi, rax\n");
            printf("    setne al\n");
            printf("    movzb rax, al\n");
            break;
        case '+':
            printf("    add rax, rdi\n");
            break;
        case '-':
            printf("    sub rax, rdi\n");
            break;
        case '*':
            printf("    mul rdi\n");
            break;
        case '/':
            printf("    mov rdx, 0\n");
            printf("    div rdi\n");
        }

    printf("    push rax\n");
}
