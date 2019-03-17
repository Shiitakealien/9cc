// indicating the type of a token
enum {
    TK_NUM = 256,   // integer token
    TK_IDENT,       // identifier token
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
extern Map *vars;
extern Node *code[100];

Vector *new_vector();
void vec_push(Vector *vec, void *elem);
Map *new_map();
void map_put(Map *map, char *key, void *val);
void *map_get(Map *map, char *key);
void add_var(Map *map, char *name);
Token *add_token(Vector *tokens, int ty, char *input);
char *add_token_var(Vector *tokens, char *p);
int isoperator(char *p);
Vector *tokenizer(char *p);
int expect(int line, int expected, int actual);
void test_vector();
void test_map() ;
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
void gen_lval(Node *node);
void gen(Node *node);


