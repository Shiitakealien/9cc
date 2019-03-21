#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// indicating the type of a token
enum TK_NUM {
    TK_NUM = 256,   // integer token
    TK_IDENT,       // identifier token
    TK_IF,          // if
    TK_ELSE,        // else
    TK_WHILE,       // while
    TK_RETURN,      // return
    TK_EQ,          // ==
    TK_EQN,         // !=
    TK_EOF,         // End Of File token
};

// Token Type
typedef struct {
    int  ty;        // Token's type
    int  val;       // Token's value, used when ty is TK_NUM
    char *input;    // Token's string
} Token;

// variable length array
typedef struct {
    void **data;        // data body
    int capacity; 
    int len;
} Vector;

// associative array 
typedef struct {
    Vector *keys;
    Vector *vals;
} Map;

// indicating the type of a node
enum ND_NUM {
    ND_NUM = 256,       // integer
    ND_IDENT,           // identifer
    ND_CALL,            // function call
    ND_IF,              // if
    ND_WHILE,           // while
    ND_RETURN,          // return
    ND_EQ,              // ==
    ND_EQN,             // !=
};

// Node Type
typedef struct Node {
    int ty;             // type -- operator or ND_NUM
    struct Node *lhs;   // left-hand side
    struct Node *rhs;   // right-hand side
    struct Node *cond;  // condition when ty is ND_IF
    int val;            // used only when ty is ND_NUM
    char *name;         // used only when ty is ND_IDENT
    Vector *args;       // arguments for ND_CALL
    int id;             // id number for jump label
} Node;

// struction of function
typedef struct {
    char   *name;       // function name
    Vector *args;       // will be used
    Node   *code[100];  // Nodes constructing definition of a function
    Map    *idents;     // local identifiers
} Function;

extern int pos; // indicates the position in all of tokens
extern Vector *tokens;

Vector *new_vector();
void vec_push(Vector *vec, void *elem);
Map *new_map();
void map_put(Map *map, char *key, void *val);
void *map_get(Map *map, char *key);
int isoperator(char *p);
Vector *tokenizer(char *p);
void runtest();
Vector *program();
void gen(Function *func, Node *node);
