#include "9cc.h"

int main(int argc, char **argv){
    if (argc != 2){
        fprintf(stderr, "usage: ./9cc [code]\n");
        return 1;
    }

    if (strcmp(argv[1] ,"-test") == 0){
        runtest();
        return 0;
    }
    // tokenize
    Vector *tokens = tokenizer(argv[1]);
    Vector *funcs = program(tokens);
    
    gen(funcs);
    return 0;
}
