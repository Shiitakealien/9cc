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

    // write the header of assembly
    printf(".intel_syntax noprefix\n");
    printf(".global");
    for (int i = 0; i < funcs->len; i++)
        printf(" %s,",((Function *)(funcs->data[i]))->name); 
    printf("\n");
    for (int i = 0; i < funcs->len; i++){
        Function *func = (Function *)(funcs->data[i]);
        printf("%s:\n",func->name);

        // function prologue
        printf("    push rbp\n");
        printf("    mov rbp, rsp\n");
        // Reserve a space for local variables
        int var_num = func->idents->keys->len; // number of local variables
        int heap = 16-((var_num*8)%16)+var_num*8;
        printf("    sub rsp, %d\n", heap);
        // copy every arg into the local variable
        char * reg[] = {"rdi","rsi","rdx","rcx","r8","r9"};
        for (int i = 0; i <= func->args->len-1; i++){
            printf("    mov rax, rbp\n");
            printf("    sub rax, %d\n",(func->args->len-i)*8);
            printf("    mov [rax], %s\n",reg[i]);
        }

        // generate a code from the head
        for (int j = 0; func->code[j]; j++){
            gen(func,func->code[j]);
        }
    }
    return 0;
}
