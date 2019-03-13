#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// トークンの型を表す値
enum {
    TK_NUM = 256,   // 整数トークン
    TK_EOF,         // 入力の終わりを表すトークン
};

// トークンの型
typedef struct {
    int ty;         // トークンの型
    int val;        // tyがTK_NUMの場合，その数値
    char *input;    // トークン文字列（エラーメッセージ用）
} Token;

// トークナイズした結果のトークン列を収納する配列
// 暫定的に最大トークン数は100個しておく
Token tokens[1000];

// pが指している文字列をトークンに分割してtokensに保存する
void tokenize(char *p) {
    int i = 0;
    while(*p){
        // 空白文字をスキップ
        if (isspace(*p)){
            p++;
            continue;
        }

        if (*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' || *p == ')' ){
            tokens[i].ty = *p;
            tokens[i].input = p;
            i++;
            p++;
            continue;
        }

        if (isdigit(*p)){
            tokens[i].ty = TK_NUM;
            tokens[i].input = p;
            tokens[i].val = strtol(p, &p, 10);
            i++;
            continue;
        }

        fprintf(stderr, "トークナイズ出来ません : %s\n", p);
        exit(1);
    }

    tokens[i].ty = TK_EOF;
    tokens[i].input = p;
}

// エラーを報告するための関数
//void error(const char *s){
//    fprintf(stderr, s);
//    exit(1);
//}

enum {
    ND_NUM = 256,
};

typedef struct Node {
    int ty;             // type -- operator or ND_NUM
    struct Node *lhs;   // left-hand side
    struct Node *rhs;   // right-hand side
    int val;            // used only when ty is ND_NUM
} Node;

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

int pos = 0;

int consume(int ty){
    if (tokens[pos].ty != ty)
        return 0;
    pos++;
    return 1;
}

Node *add();
Node *mul();
Node *term();

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
    if (consume('(')){
        Node *node = add();
        if (!consume(')')){
            //error("開き括弧に対応する閉じ括弧がない: %s", tokens[pos].input);
            fprintf(stderr,"開き括弧に対応する閉じ括弧がない: %s", tokens[pos].input);
            exit(1);
        }
        return node;
    }

    if (tokens[pos].ty == TK_NUM)
        return new_node_num(tokens[pos++].val);

    //error("数値でも開き括弧でもないトークンです: %s", tokens[pos].input);
}

void gen(Node *node){
    if (node->ty == ND_NUM){
        printf("    push %d\n", node->val);
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

    // トークナイズする
    tokenize(argv[1]);
    Node *node = add();

    // アセンブリの前半部分を出力する
    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

    // decent the tree and generate a code
    gen(node);

    // We have the result at the top of the stack
    printf("    pop rax\n");
    printf("    ret\n");
    return 0;
}
