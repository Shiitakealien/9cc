#include "9cc.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

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
    while (('a' <= *p && *p <= 'z') || 
           ('A' <= *p && *p <= 'Z' ) || isdigit(*p))
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
        *p == '=' || *p == ';' || *p == '!')
        return 1;
    return 0;
}

// separate a string pointed by 'p' and save in 'tokens'
Vector *tokenizer(char *p) {
    Vector *vec = new_vector();
    Token *token = malloc(sizeof(Token));
    while(*p){
        // skip blanc
        if (isspace(*p)){
            p++;
            continue;
        }

        if (isoperator(p)){
            if (*p == '=' && *p+1 == '='){
                add_token(vec, TK_EQ, p);
                p+=2;
            } else if (*p == '!' && *p+1 == '=') {
                add_token(vec, TK_EQN, p);
                p+=2;
            } else {
            add_token(vec, *p, p);
            p++;
            }
            continue;
        }

        if (('a' <= *p && *p <= 'z') || 
            ('A' <= *p && *p <= 'Z')){
            p = add_token_var(vec, p);
            continue;
        }

        if (isdigit(*p)){
            token = add_token(vec, TK_NUM, p);
            token->val = strtol(p, &p, 10);
            continue;
        }

        fprintf(stderr, "cannot tokenize : %s\n", p);
        exit(1);
    }
    add_token(vec, TK_EOF, p);
    return vec;
}


