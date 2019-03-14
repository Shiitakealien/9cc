#include <ctype.h>
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

// variable length array
typedef struct {
    void **data; // data body
    int capacity; 
    int len;
} Vector;

enum {
    ND_NUM = 256,
};

typedef struct Node {
    int ty;             // type -- operator or ND_NUM
    struct Node *lhs;   // left-hand side
    struct Node *rhs;   // right-hand side
    int val;            // used only when ty is ND_NUM
} Node;

Vector *new_vector();
void vec_push(Vector *vec, void *elem);
void tokenize(char *p) ;
int expect(int line, int expected, int actual);
void runtest();
Node *new_node(int ty, Node *lhs, Node *rhs);
Node *new_node_num(int val);
int consume(int ty);
Node *add();
Node *mul();
Node *term();
void gen(Node *node);
