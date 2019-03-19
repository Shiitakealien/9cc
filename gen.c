#include "9cc.h"

static void gen_lval(Function *func, Node *node){
    if (node->ty != ND_IDENT){
        fprintf(stderr, "lhs is not a variable");
        exit(1);
    }

    int offset = (int)map_get(func->idents, node->name) * 8;
    printf("    mov rax, rbp\n");
    printf("    sub rax, %d\n", offset);
    printf("    push rax\n");
}

void gen(Function *func, Node *node){
    if (node->ty == ND_NUM){
        printf("    push %d\n", node->val);
        return;
    }

    if (node->ty == ND_IDENT){
        gen_lval(func, node);
        printf("    pop rax\n");
        printf("    mov rax, [rax]\n");
        printf("    push rax\n");
        return;
    }

    if (node->ty == ND_CALL){
        char * reg[] = {"rdi","rsi","rdx","rcx","r8","r9"};
        for (int i = node->args->len-1; i >= 0; i--){
            gen(func, (Node *)(node->args->data[i]));
            printf("    pop %s\n",reg[i]);
        }
        printf("    call %s\n", node->name);
        printf("    push rax\n");
        return;
    }

    if (node->ty == '='){
        gen_lval(func, node->lhs);
        gen(func, node->rhs);

        printf("    pop rdi\n");
        printf("    pop rax\n");
        printf("    mov [rax], rdi\n");
        printf("    push rdi\n");
        return;
    }
        
    gen(func, node->lhs);
    gen(func, node->rhs);

    printf("    pop rdi\n");
    printf("    pop rax\n");

    switch (node->ty){
        case ND_EQ:
            printf("    cmp rdi, rax\n");
            printf("    sete al\n");
            printf("    movzb rax, al\n");
            break;
        case ND_EQN:
            printf("    cmp rdi, rax\n");
            printf("    setne al\n");
            printf("    movzb rax, al\n");
            break;
        case '+':
            printf("    add rax, rdi\n");
            break;
        case '-':
            printf("    sub rax, rdi\n");
            break;
        case '*':
            printf("    mul rdi\n");
            break;
        case '/':
            printf("    mov rdx, 0\n");
            printf("    div rdi\n");
        }

    printf("    push rax\n");
}
