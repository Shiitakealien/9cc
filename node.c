#include "icc.h"

static int pos = 0;
static Vector *tokens;
static Function *func;

static Node *cond();
static Node *stmt();
static Node *assign();
static Node *eq();
static Node *add();
static Node *mul();
static Node *term();

static Token *crnt_tkn(){
    return (Token *)(tokens->data[pos]);
}

static Node *new_node(int ty, Node *lhs, Node *rhs){
    Node *node = malloc(sizeof(Node));
    node->ty = ty;
    node->lhs = lhs;
    node->rhs = rhs;
    if (ty==ND_REF) {
        node->eval = lhs->eval->ptrof;
    } else if (ty==ND_ADDR) {
        Var *v = malloc(sizeof(Var));
        v->ty    = PTR;  // is this right?
        v->ptrof = lhs->eval;
        node->eval = v;
    } else if (ty=='*' || ty=='/') {
        if (lhs->eval->ty == PTR || rhs->eval->ty == PTR) {
            fprintf(stderr,"This operation is not allowed\n");
            exit(1);
        }
        Var *v = malloc(sizeof(Var));
        v->ty    = INT;
        node->eval = v;
    } else if (ty=='+' || ty=='-') {
        if (lhs->eval->ty == PTR && rhs->eval->ty == PTR) {
            fprintf(stderr,"PTR %c PTR is not allowed\n", ty);
            exit(1);
        } else if (lhs->eval->ty == PTR) {
            node->eval = lhs->eval;
        } else if (rhs->eval->ty == PTR) {
            node->eval = rhs->eval;
        } else  { // lhs and rhs are INT
            node->eval = rhs->eval;
        }
    } else if (ty==ND_EQ || ty==ND_EQN || ty=='>' || 
               ty==ND_GE || ty=='<'    || ty==ND_LE || ty=='=') {
        ; // will be added comparing the evaluation
    } 
    return node;
}

static Node *new_node_term(int ty, int val, char *input){
    Node *node = malloc(sizeof(Node));
    node->ty = ty;
    node->val = val;
    node->name = input;
    if (ty==ND_NUM || ty==ND_CALL) {
        Var *v = malloc(sizeof(Var));
        v->ty = INT;
        node->eval = v;
    } else if (ty==ND_IDENT) {
        node->eval = (Var *)(map_get(func->idents, input));
    }
    return node;
}

static int consume(int ty){
    if (crnt_tkn()->ty != ty)
        return 0;
    pos++;
    return 1;
}

static int expect(int ty){
    if (consume(ty))
        return 1;
    else {
        int exp = ty;
        int res = crnt_tkn()->ty;
        char *res_c = crnt_tkn()->input;
        fprintf(stderr,
                "token error: expect %d( '%c'), ",
                exp, exp);
        fprintf(stderr,
                "got %d( '%s') at %dth token\n", 
                res, res_c, pos);
        exit(1);
    }
}

static void add_ident(Map *map, char *name, int ptr_depth){
    if (!map_exists(map, name)){
        int i;
        Var *v[ptr_depth];
        v[ptr_depth] = malloc(sizeof(Var));
        v[ptr_depth]->ty = INT;
        for(i=ptr_depth-1;i>=0;i--){
            v[i] = (Var *)malloc(sizeof(Var));
            v[i]->ty = PTR;
            v[i]->ptrof=v[i+1];
        }
        v[0]->offset = map->keys->len;
        map_put(map, name, (void *)v[0]);
    }
}

static Function *add_func(Vector *funcs, char *name){
    Function *func = malloc(sizeof(Function));
    func->name = name;
    func->args = new_vector();
    func->idents = new_map();
    vec_push(funcs, (void *)func);
    return func;
}

static Node *cond_node(int ty){
    int id = pos - 1; // token number of "if"
    expect('(');
    Node *cond_n = assign();
    expect(')');
    Node *node = new_node(ty, cond(), (Node *)NULL);
    node->cond=cond_n;
    node->id=id;
    if (consume(TK_ELSE) && ty == TK_IF)
        node->rhs = cond();
    return node;
}

static Node *cond(){
    int ptr_depth;
    if (consume(TK_INT)){
        ptr_depth = 0;
        while(consume('*'))
            ptr_depth++;
        add_ident(func->idents,crnt_tkn()->input,ptr_depth);
        expect(TK_IDENT);
        expect(';');
        return new_node(ND_NOP, (Node *)NULL, (Node *)NULL);
    } else if (consume(TK_RETURN))
        return new_node(ND_RETURN, stmt(), (Node *)NULL);
    else if (consume(TK_IF))
        return cond_node(ND_IF);
    else if (consume(TK_WHILE))
        return cond_node(ND_WHILE);
    else if (consume(TK_FOR)){
        int id = pos - 1; // token number of "if"
        expect('(');
        Node *init;
        if (crnt_tkn()->ty == ';')
            init = (Node *)NULL;
        else
            init = assign();
        expect(';');
        Node *cond_n;
        if (crnt_tkn()->ty == ';')
            cond_n = new_node_term(ND_NUM,1,crnt_tkn()->input);
        else
            cond_n = assign();
        expect(';');
        Node *next;
        if (crnt_tkn()->ty == ')')
            next = (Node *)NULL;
        else
            next = assign();
        expect(')');
        Node *loop = stmt();
        Node *for_node = new_node(ND_FOR, loop, next);
        for_node->cond = cond_n;
        for_node->id=id;
        return new_node(ND_NOP, init, for_node);
    } else
        return stmt();
}

static Node *stmt(){
    Node *node = (Node *)NULL;
    if (consume(';'))
        return new_node(ND_NOP, (Node *)NULL, (Node *)NULL);
    else if (consume('{')){
        while(!consume('}')) // generate node at every loop
            node = new_node(ND_COMP, node, stmt());
        return node;
    } else {
        node = assign();
        expect(';');
        return node;
    }
}   

static Node *assign(){
    Node *node = eq();
    for (;;)
        if (consume('='))
            node = new_node('=', node, assign());
        else
            return node;
}

static Node *eq(){
    Node *node = add();
    for (;;)
        if (consume(TK_EQ))
            node = new_node(ND_EQ, node, add());
        else if (consume(TK_EQN))
            node = new_node(ND_EQN, node, add());
        else if (consume('>'))
            node = new_node('>', node, add());
        else if (consume(TK_GE))
            node = new_node(ND_GE, node, add());
        else if (consume('<'))
            node = new_node('<', node, add());
        else if (consume(TK_LE))
            node = new_node(ND_LE, node, add());
        else
            return node;
}

static Node *add(){
    Node *node = mul();
    for (;;)
        if (consume('+'))
            node = new_node('+', node, mul());
        else if (consume('-'))
            node = new_node('-', node, mul());
        else
            return node;
}

static Node *mul(){
    Node *node = term();
    for (;;)
        if (consume('*'))
            node = new_node('*', node, term());
        else if (consume('/'))
            node = new_node('/', node, term());
        else
            return node;
}
    
static Node *term(){
    Token *t = (Token *)(tokens->data[pos]);

    if (consume('*'))
        return new_node(ND_REF, term(), (Node *)NULL);

    if (consume('&'))
        return new_node(ND_ADDR, term(), (Node *)NULL);

    if (consume('(')){
        Node *node = add();
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
                     vec_push(node->args, (void *)assign());
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
    int code_num, ptr_depth;
    Vector *funcs = new_vector();
    while (crnt_tkn()->ty != TK_EOF){
        if (consume(TK_INT)){
            code_num = 0;
            func = add_func(funcs, crnt_tkn()->input);
            expect(TK_IDENT);
            expect('(');
            for(;;){
                if(consume(')'))  // no arguments
                    break;
                expect(TK_INT);
                ptr_depth = 0;
                while(consume('*'))
                    ptr_depth++;
                add_ident(func->idents,crnt_tkn()->input,ptr_depth);
                vec_push(func->args,(void *)crnt_tkn()->input);
                pos++;
                consume(',');
            }
            expect('{');
            while (crnt_tkn()->ty != '}')
                func->code[code_num++] = cond(func);
            func->code[code_num] = NULL;
            expect('}');
        }
    }
    return funcs;
}
