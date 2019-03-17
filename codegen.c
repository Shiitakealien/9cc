#include "9cc.h"
#include <stdio.h>
#include <stdlib.h>

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

Node *new_node_func(char *input){
    Node *node = malloc(sizeof(Node));
    node->ty = ND_FUNC;
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
    Node *node = eq();
    
    for (;;){
        if (consume('='))
            node = new_node('=', node, assign());
        else
            return node;
    }
}

Node *eq(){
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

    if (consume(TK_FUNC))
        return new_node_func(token->input);

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

    if (node->ty == ND_FUNC){
        printf("    call %s\n", node->name);
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
