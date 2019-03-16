#include <ctype.h>
#include <string.h>

// トークンの型を表す値
enum {
    TK_NUM = 256,   // 整数トークン
    TK_IDENT,       // identifier
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
    ND_NUM = 256,       // Node type of integer
    ND_IDENT,           // Node type of identifer
};

typedef struct Node {
    int ty;             // type -- operator or ND_NUM
    struct Node *lhs;   // left-hand side
    struct Node *rhs;   // right-hand side
    int val;            // used only when ty is ND_NUM
    char name;          // used only when ty is ND_IDENT
} Node;

Vector *new_vector();
void vec_push(Vector *vec, void *elem);
Token *add_token(Vector *tokens, int ty, char *input);
Vector *tokenizer(char *p) ;
int expect(int line, int expected, int actual);
void runtest();
Node *new_node(int ty, Node *lhs, Node *rhs);
Node *new_node_num(int val);
Node *new_node_ident(char *input);
int consume(int ty);
void program();
Node *stmt();
Node *assign();
Node *add();
Node *mul();
Node *term();
void gen(Node *node);
