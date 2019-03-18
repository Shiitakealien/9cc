#include "9cc.h"

static Node *stmt();
static Node *assign();
static Node *eq();
static Node *add();
static Node *mul();
static Node *term();

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

void program(){
    int i = 0;
    while (((Token *)(tokens->data[pos]))->ty != TK_EOF)
        code[i++] = stmt();
    code[i] = NULL;
}

static Node *stmt(){
    Token *token = (Token *)(tokens->data[pos]);
    Node *node = assign();
    if (!consume(';')){
        fprintf(stderr,"';'ではないトークンです: %s", token->input);
        exit(1);
    }
    return node;
}        

static Node *assign(){
    Node *node = eq();
    
    for (;;){
        if (consume('='))
            node = new_node('=', node, assign());
        else
            return node;
    }
}

static Node *eq(){
    Node *node = add();

    for (;;){
        if (consume(TK_EQ))
            node = new_node(ND_EQ, node, add());
        else if (consume(TK_EQN))
            node = new_node(ND_EQN, node, add());
        else
            return node;
    }
}

static Node *add(){
    Node *node = mul();
    
    for (;;){
        if (consume('+'))
            node = new_node('+', node, mul());
        else if (consume('-'))
            node = new_node('-', node, mul());
        else
            return node;
    }
}

static Node *mul(){
    Node *node = term();

    for (;;){
        if (consume('*'))
            node = new_node('*', node, term());
        else if (consume('/'))
            node = new_node('/', node, term());
        else
            return node;
    }
}
    
static Node *term(){
    Token *token = (Token *)(tokens->data[pos]);

    if (consume('(')){
        Node *node = add();
        if (!consume(')')){
            fprintf(stderr,"'(' without ')': %s", 
                    token->input);
            exit(1);
        }
        return node;
    }

    if (consume(TK_NUM))
        return new_node_num(token->val);
    
    if (consume(TK_IDENT))
        if (!consume('('))
            return new_node_ident(token->input);
        else if (consume(')'))
            return new_node_call(token->input);

    fprintf(stderr,"found an unknown token: %s", token->input);
    exit(1);
}

void gen_lval(Node *node){
    if (node->ty != ND_IDENT){
        fprintf(stderr, "lhs is not a variable");
        exit(1);
    }

    int offset = (int)map_get(idents, node->name) * 8;
    printf("    mov rax, rbp\n");
    printf("    sub rax, %d\n", offset);
    printf("    push rax\n");
}

void gen(Node *node){
    if (node->ty == ND_NUM){
        printf("    push %d\n", node->val);
        return;
    }

    if (node->ty == ND_IDENT){
        gen_lval(node);
        printf("    pop rax\n");
        printf("    mov rax, [rax]\n");
        printf("    push rax\n");
        return;
    }

    if (node->ty == ND_CALL){
        printf("    mov rax, 0\n");
        printf("    call %s\n", node->name);
        printf("    push rax\n");
        return;
    }

    if (node->ty == '='){
        gen_lval(node->lhs);
        gen(node->rhs);

        printf("    pop rdi\n");
        printf("    pop rax\n");
        printf("    mov [rax], rdi\n");
        printf("    push rdi\n");
        return;
    }
        
    gen(node->lhs);
    gen(node->rhs);

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
