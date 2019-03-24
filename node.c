#include "9cc.h"

static int pos = 0;
static Vector *tokens;

static Node *cond(Function *func);
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

static Node *new_node_term(int ty, int val, char *input){
    Node *node = malloc(sizeof(Node));
    node->ty = ty;
    node->val = val;
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

static int expect(int ty){
    if (consume(ty))
        return 1;
    else {
        int exp = ((Token *)(tokens->data[pos]))->ty;
        int res = ty;
        char *res_c = ((Token *)(tokens->data[pos]))->input;
        fprintf(stderr,
                "token error: expect %d, got %d( '%s') at %dth token\n", 
                exp, res, res_c, pos);
        exit(1);
    }
}

static void add_ident(Map *map, char *name){
    if (!map_exists(map, name))
        map_put(map, name, (void *)(intptr_t)(map->keys->len));
}

static Function *add_func(Vector *funcs, char *name){
    Function *func = malloc(sizeof(Function));
    func->name = name;
    func->args = new_vector();
    func->idents = new_map();
    vec_push(funcs, (void *)func);
    return func;
}

static Token *current_token(){
    return (Token *)(tokens->data[pos]);
}

static Node *cond_node(Function *func, int ty){
    int id = pos - 1; // token number of "if"
    expect('(');
    Node *cond_n = assign(func);
    expect(')');
    Node *node = new_node(ty, cond(func), (Node *)NULL);
    node->cond=cond_n;
    node->id=id;
    if (consume(TK_ELSE) && ty == TK_IF)
        node->rhs = cond(func);
    return node;
}

static Node *cond(Function *func){
    if (consume(TK_INT)){
        add_ident(func->idents, ((Token *)(tokens->data[pos]))->input);
        expect(TK_IDENT);
        expect(';');
        return new_node(ND_NOP, (Node *)NULL, (Node *)NULL);
    } else if (consume(TK_RETURN))
        return new_node(ND_RETURN, stmt(func), (Node *)NULL);
    else if (consume(TK_IF))
        return cond_node(func, ND_IF);
    else if (consume(TK_WHILE))
        return cond_node(func, ND_WHILE);
    else if (consume(TK_FOR)){
        int id = pos - 1; // token number of "if"
        expect('(');
        Token *t = current_token();
        Node *init;
        if (t->ty == ';')
            init = (Node *)NULL;
        else
            init = assign(func);
        expect(';');
        Node *cond_n;
        t = current_token();
        if (t->ty == ';')
            cond_n = new_node_term(ND_NUM,1,t->input);
        else
            cond_n = assign(func);
        expect(';');
        Node *next;
        t = current_token();
        if (t->ty == ')')
            next = (Node *)NULL;
        else
            next = assign(func);
        expect(')');
        Node *loop = stmt(func);
        Node *for_node = new_node(ND_FOR, loop, next);
        for_node->cond = cond_n;
        for_node->id=id;
        return new_node(ND_NOP, init, for_node);
    } else
        return stmt(func);
}

static Node *stmt(Function *func){
    Node *node = (Node *)NULL;
    if (consume(';'))
        return new_node(ND_NOP, (Node *)NULL, (Node *)NULL);
    else if (consume('{')){
        while(!consume('}')) // generate node at every loop
            node = new_node(ND_NOP, node, stmt(func));
        return node;
    } else {
        node = assign(func);
        expect(';');
        return node;
    }
}   

static Node *assign(Function *func){
    Node *node = eq(func);
    for (;;)
        if (consume('='))
            node = new_node('=', node, assign(func));
        else
            return node;
}

static Node *eq(Function *func){
    Node *node = add(func);
    for (;;)
        if (consume(TK_EQ))
            node = new_node(ND_EQ, node, add(func));
        else if (consume(TK_EQN))
            node = new_node(ND_EQN, node, add(func));
        else if (consume('>'))
            node = new_node('>', node, add(func));
        else if (consume(TK_GE))
            node = new_node(ND_GE, node, add(func));
        else if (consume('<'))
            node = new_node('<', node, add(func));
        else if (consume(TK_LE))
            node = new_node(ND_LE, node, add(func));
        else
            return node;
}

static Node *add(Function *func){
    Node *node = mul(func);
    for (;;)
        if (consume('+'))
            node = new_node('+', node, mul(func));
        else if (consume('-'))
            node = new_node('-', node, mul(func));
        else
            return node;
}

static Node *mul(Function *func){
    Node *node = term(func);
    for (;;)
        if (consume('*'))
            node = new_node('*', node, term(func));
        else if (consume('/'))
            node = new_node('/', node, term(func));
        else
            return node;
}
    
static Node *term(Function *func){
    Token *t = (Token *)(tokens->data[pos]);

    if (consume('(')){
        Node *node = add(func);
        if (!consume(')')){
            fprintf(stderr,"'(' without ')': %s", 
                    t->input);
            exit(1);
        }
        return node;
    }

    if (consume(TK_NUM))
        return new_node_term(ND_NUM,t->val,t->input);
    
    if (consume(TK_IDENT)){
        if (!consume('(')){
            return new_node_term(ND_IDENT,t->val,t->input);
        }
        else {
            Node *node = new_node_term(ND_CALL,t->val,t->input);
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

    fprintf(stderr,"found an unknown token: %s\n", t->input);
    exit(1);
}

Vector *program(Vector *arg_tokens){
    tokens = arg_tokens;
    int i;
    Token *t = current_token();
    Function *func;
    Vector *funcs = new_vector();
    while (t->ty != TK_EOF){
        if (consume(TK_INT)){
            i = 0;
            t = current_token();
            func = add_func(funcs, t->input);
            expect(TK_IDENT);
            expect('(');
            for(;;){
                if(consume(')'))  // no arguments
                    break;
                expect(TK_INT);
                t = current_token();
                add_ident(func->idents,t->input);
                vec_push(func->args,(void *)t->input);
                pos++;
                consume(',');
            }
            expect('{');
            t = current_token();
            while (t->ty != '}'){
                func->code[i++] = cond(func);
                t = current_token();
            }
            func->code[i] = NULL;
            expect('}');
            t = current_token();
        }
    }
    return funcs;
}
