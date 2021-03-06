#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>

// indicating the type of a token
enum TK_NUM {
    TK_NUM = 256,   // integer token
    TK_IDENT,       // identifier token
    TK_INT,         // int
    TK_IF,          // if
    TK_ELSE,        // else
    TK_WHILE,       // while
    TK_RETURN,      // return
    TK_FOR,         // return
    TK_EQ,          // ==
    TK_EQN,         // !=
    TK_GE,          // >=
    TK_LE,          // <=
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
    ND_FOR,             // for
    ND_RETURN,          // return
    ND_REF,             // * for pointers
    ND_ADDR,            // & for pointers
    ND_EQ,              // ==
    ND_EQN,             // !=
    ND_GE,              // >=
    ND_LE,              // <=
    ND_COMP,            // compound statements
    ND_NOP,             // NOP
};

// Node Type
typedef struct Node {
    int ty;             // type -- operator or ND_NUM
    struct Var *eval;   // evaluation of Node
    struct Node *lhs;   // left-hand side
    struct Node *rhs;   // right-hand side
    struct Node *cond;  // condition when ty is ND_IF, ND_WHILE, ND_FOR
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
    int    var_sum;     // sum of local variable memory space
} Function;

// variable type
typedef struct Var {
    enum { INT, PTR } ty;
    struct Var *ptrof;  // used when TY = PTR
    int offset;         // relative addr for RBP
} Var;

Vector *new_vector();
void vec_push(Vector *vec, void *elem);
Map *new_map();
void map_put(Map *map, char *key, void *val);
void *map_get(Map *map, char *key);
int map_exists(Map *map, char *key);
Vector *tokenizer(char *p);
void runtest();
Vector *program(Vector *arg_tokens);
void gen(Vector *funcs);
