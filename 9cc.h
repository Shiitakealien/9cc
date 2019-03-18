#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// indicating the type of a token
enum {
    TK_NUM = 256,   // integer token
    TK_IDENT,       // identifier token
    TK_EQ,          // ==
    TK_EQN,         // !=
    TK_EOF,         // End Of File token
};

// Token Type
typedef struct {
    int ty;         // Token's type
    int val;        // Token's value, used when ty is TK_NUM
    char *input;    // Token's string
} Token;

// variable length array
typedef struct {
    void **data;        // data body
    int capacity; 
    int len;
} Vector;

// indicating the type of a node
enum {
    ND_NUM = 256,       // Node type of integer
    ND_IDENT,           // Node type of identifer
    ND_CALL,            // Node type of function call
    ND_EQ,              // Node type of ==
    ND_EQN,             // Node type of !=
};

// Node Type
typedef struct Node {
    int ty;             // type -- operator or ND_NUM
    struct Node *lhs;   // left-hand side
    struct Node *rhs;   // right-hand side
    int val;            // used only when ty is ND_NUM
    char *name;         // used only when ty is ND_IDENT
} Node;

// associative array 
typedef struct {
    Vector *keys;
    Vector *vals;
} Map;

extern int pos;
extern Vector *tokens;
extern Map *idents;
extern Node *code[100];

Vector *new_vector();
void vec_push(Vector *vec, void *elem);
Map *new_map();
void map_put(Map *map, char *key, void *val);
void *map_get(Map *map, char *key);
int isoperator(char *p);
Vector *tokenizer(char *p);
void runtest();
void program();
void gen_lval(Node *node);
void gen(Node *node);


