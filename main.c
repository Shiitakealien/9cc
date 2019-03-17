#include "9cc.h"
#include <stdio.h>
#include <string.h>

int pos = 0;
Vector *tokens;
Map *vars;
Node *code[100];

int main(int argc, char **argv){
    if (argc != 2){
        fprintf(stderr, "引数の数が不正です\n");
        return 1;
    }

    if (strcmp(argv[1] ,"-test") == 0)
        runtest();
    else{
        vars = new_map();
        // tokenize
        tokens = tokenizer(argv[1]);
        program();

        // write the header of assembly
        printf(".intel_syntax noprefix\n");
        printf(".global main\n");
        printf("main:\n");

        // prologue
        printf("    push rbp\n");
        printf("    mov rbp, rsp\n");
        printf("    sub rsp, %d\n", vars->keys->len*8);

        // generate a code from the head
        for (int i = 0; code[i]; i++){
            gen(code[i]);
            printf("    pop rax\n");
        }
        // We have the result at the top of the stack
        printf("    mov rsp, rbp\n");
        printf("    pop rbp\n");
        printf("    ret\n");
    }
    return 0;
}

