#include "9cc.h"
#include <stdio.h>
#include <stdlib.h>

int pos = 0;
Vector *tokens;
Map *vars;

Vector *new_vector(){
    Vector *vec = malloc(sizeof(Vector));
    vec->data = malloc(sizeof(void *) * 16);
    vec->capacity = 16;
    vec->len = 0;
    return vec;
}

void vec_push(Vector *vec, void *elem){
    if (vec->capacity == vec->len){
        vec->capacity *= 2;
        vec->data = realloc(vec->data, sizeof(void *) * vec->capacity);
    }
    vec->data[vec->len++] = elem;
}

Map *new_map(){
    Map *map = malloc(sizeof(Map));
    map->keys = new_vector();
    map->vals = new_vector();
    return map;
}

void map_put(Map *map, char *key, void *val){
    vec_push(map->keys, key);
    vec_push(map->vals, val);
}

void *map_get(Map *map, char *key){
    for (int i = map->keys->len - 1; i >= 0; i--)
        if (strcmp(map->keys->data[i], key) == 0)
            return map->vals->data[i];
    return NULL;
}
    
void add_var(Map *map, char *name){
    if (map_get(map, name) == NULL)
        map_put(map, name, (void *)(map->keys->len));
}

Token *add_token(Vector *tokens, int ty, char *input){
    Token *token = malloc(sizeof(Token));
    token->ty = ty;
    token->input = input;
    vec_push(tokens,token);
    return token;
}

char *add_token_var(Vector *tokens, char *p){
    char *varname = malloc(sizeof(char) * 256);
    int i = 0;
    while (('a' <= *p && *p <= 'z') || isdigit(*p))
        *(varname+i++) = *p++;
    *(varname+i) = '\0';
    add_var(vars, varname);
    add_token(tokens, TK_IDENT, varname);
    return p;
}

int isoperator(char *p){
    if (*p == '+' || *p == '-' || 
        *p == '*' || *p == '/' || 
        *p == '(' || *p == ')' ||
        *p == '=' || *p == ';' )
        return 1;
    return 0;
}

// separate a string pointed by 'p' and save in 'tokens'
Vector *tokenizer(char *p) {
    Vector *vec = new_vector();
    Token *token = malloc(sizeof(Token));
    int i = 0;
    while(*p){
        // skip blanc
        if (isspace(*p)){
            p++;
            continue;
        }

        if (isoperator(p)){
            add_token(vec, *p, p);
            p++;
            continue;
        }

        if ('a' <= *p && *p <= 'z'){
            p = add_token_var(vec, p);
            continue;
        }

        if (isdigit(*p)){
            token = add_token(vec, TK_NUM, p);
            token->val = strtol(p, &p, 10);
            continue;
        }

        fprintf(stderr, "トークナイズ出来ません : %s\n", p);
        exit(1);
    }
    add_token(vec, TK_EOF, p);
    return vec;
}

int expect(int line, int expected, int actual){
    if (expected == actual)
        return 1;
    fprintf(stderr, "%d: %d expected, but got %d\n",
                    line, expected, actual);
    exit(1);
    return 0;
}

void test_vector(){
    printf("test vector\n");
    Vector *vec = new_vector();
    expect(__LINE__, 0, vec->len);
    int test[100];
    for (int i = 0; i <100; i++){
        test[i] = i;
        vec_push(vec, (void *)&test[i]);
    }

    expect(__LINE__, 100, vec->len);
    expect(__LINE__, 0,   *(int *)(vec->data[0]));
    expect(__LINE__, 50,  *(int *)(vec->data[50]));
    expect(__LINE__, 99,  *(int *)(vec->data[99]));

    printf("OK\n");
}

void test_map() {
    printf("test map\n");
    int test_data[] = {2,4,6};
    Map *map = new_map();

    map_put(map, "foo", (void *)&test_data[0]);
    expect(__LINE__, 2, *(int *)map_get(map, "foo"));

    map_put(map, "bar", (void *)&test_data[1]);
    expect(__LINE__, 4, *(int *)map_get(map, "bar"));
    
    map_put(map, "foo", (void *)&test_data[2]);
    expect(__LINE__, 6, *(int *)map_get(map, "foo"));
    
    printf("OK\n");
}

void runtest(){
    test_vector();
    test_map();
}

Node *new_node(int ty, Node *lhs, Node *rhs){
    Node *node = malloc(sizeof(Node));
    node->ty = ty;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

Node *new_node_num(int val){
    Node *node = malloc(sizeof(Node));
    node->ty = ND_NUM;
    node->val = val;
    return node;
}

Node *new_node_ident(char *input){
    Node *node = malloc(sizeof(Node));
    node->ty = ND_IDENT;
    node->name = input;
    return node;
}

int consume(int ty){
    Token *token = (Token *)(tokens->data[pos]);
    if (token->ty != ty)
        return 0;
    pos++;
    return 1;
}

Node *code[100];

void program(){
    int i = 0;
    while (((Token *)(tokens->data[pos]))->ty != TK_EOF)
        code[i++] = stmt();
    code[i] = NULL;
}

Node *stmt(){
    Token *token = (Token *)(tokens->data[pos]);
    Node *node = assign();
    if (!consume(';')){
        fprintf(stderr,"';'ではないトークンです: %s", token->input);
        exit(1);
    }
    return node;
}        

Node *assign(){
    Node *node = add();
    
    for (;;){
        if (consume('='))
            node = new_node('=', node, assign());
        else
            return node;
    }
}

Node *add(){
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

Node *mul(){
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
    
Node *term(){
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
        return new_node_ident(token->input);

    fprintf(stderr,"found an unknown token: %s", token->input);
    exit(1);
}

void gen_lval(Node *node){
    if (node->ty != ND_IDENT){
        fprintf(stderr, "lhs is not a variable");
        exit(1);
    }

    int offset = (int)map_get(vars, node->name) * 8;
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

int main(int argc, char **argv){
    if (argc != 2){
        fprintf(stderr, "引数の数が不正です\n");
        return 1;
    }

    if (strcmp(argv[1] ,"-test") == 0)
        runtest();
    else{
        vars = new_map();
        // tokenize
        tokens = tokenizer(argv[1]);
        program();

        // アセンブリの前半部分を出力する
        printf(".intel_syntax noprefix\n");
        printf(".global main\n");
        printf("main:\n");

        // prologue
        printf("    push rbp\n");
        printf("    mov rbp, rsp\n");
        printf("    sub rsp, %d\n", vars->keys->len*8);

        // generate a code from the head
        for (int i = 0; code[i]; i++){
            gen(code[i]);
            printf("    pop rax\n");
        }
        // We have the result at the top of the stack
        printf("    mov rsp, rbp\n");
        printf("    pop rbp\n");
        printf("    ret\n");
    }
    return 0;
}
