#include "9cc.h"

static void gen_main(Node *node);
static Function *f;

static void gen_lval(Node *node){
    if (node->ty != ND_IDENT){
        fprintf(stderr, "lhs is not a variable");
        exit(1);
    }

    Var *v = (Var *)(map_get(f->idents, node->name));
    int offset = v->offset * 8 + 8;
    printf("    mov rax, rbp\n");
    printf("    sub rax, %d\n", offset);
    printf("    push rax\n");
}

static void gen_if(Node *node){
    gen_main(node->cond);
    printf("    pop rax\n");
    printf("    cmp rax, 0\n");
    printf("    je .Lif%d\n",node->id);
    gen_main(node->lhs);
    printf("    jmp .Lelse%d\n",node->id);
    printf(".Lif%d:\n",node->id);
    if(node->rhs != NULL){
        gen_main(node->rhs);
    }
    printf(".Lelse%d:\n",node->id);
}

static void gen_while(Node *node){
    printf(".LwhileBegin%d:\n",node->id);
    gen_main(node->cond);
    printf("    pop rax\n");
    printf("    cmp rax, 0\n");
    printf("    je .LwhileEnd%d\n",node->id);
    gen_main(node->lhs);
    printf("    jmp .LwhileBegin%d\n",node->id);
    printf(".LwhileEnd%d:\n",node->id);
}

static void gen_for(Node *node){
    printf(".LforBegin%d:\n",node->id);
    gen_main(node->cond);
    printf("    pop rax\n");
    printf("    cmp rax, 0\n");
    printf("    je .LforEnd%d\n",node->id);
    gen_main(node->lhs);
    gen_main(node->rhs);
    printf("    jmp .LforBegin%d\n",node->id);
    printf(".LforEnd%d:\n",node->id);
}

// binary operation
static void gen_bin(Node *node){
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
        case '>':
            printf("    cmp rdi, rax\n");
            printf("    setl al\n");
            printf("    movzb rax, al\n");
            break;
        case ND_GE:
            printf("    cmp rdi, rax\n");
            printf("    setle al\n");
            printf("    movzb rax, al\n");
            break;
        case '<':
            printf("    cmp rax, rdi\n");
            printf("    setl al\n");
            printf("    movzb rax, al\n");
            break;
        case ND_LE:
            printf("    cmp rax, rdi\n");
            printf("    setle al\n");
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

void gen_main(Node *node){
    if (node == (Node *)NULL)
        return;
    else if (node->ty == ND_IF)
        gen_if(node);
    else if (node->ty == ND_WHILE)
        gen_while(node);
    else if (node->ty == ND_FOR)
        gen_for(node);
    else if (node->ty == ND_RETURN){
        gen_main(node->lhs);
        printf("    pop rax\n");
        printf("    mov rsp, rbp\n");
        printf("    pop rbp\n");
        printf("    ret\n");
    } else if (node->ty == ND_NUM){
        printf("    push %d\n", node->val);
        return;
    } else if (node->ty == ND_IDENT){
        gen_lval(node);
        printf("    pop rax\n");
        printf("    mov rax, [rax]\n");
        printf("    push rax\n");
        return;
    } else if (node->ty == ND_CALL){
        char * reg[] = {"rdi","rsi","rdx","rcx","r8","r9"};
        int n = node->args->len-1;
        for (int i = n; i >= 0; i--){
            gen_main((Node *)(node->args->data[n-i]));
            printf("    pop %s\n",reg[i]);
        }
        printf("    call %s\n", node->name);
        printf("    push rax\n");
        return;
    } else if (node->ty == '='){
        gen_lval(node->lhs);
        gen_main(node->rhs);
        printf("    pop rdi\n");
        printf("    pop rax\n");
        printf("    mov [rax], rdi\n");
        printf("    push rdi\n");
        return;
    } else { // binary operation or nop Node
        gen_main(node->lhs);
        gen_main(node->rhs);
        if (node->ty == ND_NOP)
            return;
        gen_bin(node);
    }
}

void gen(Vector *funcs){
    printf(".intel_syntax noprefix\n");
    printf(".global");
    for (int i = 0; i < funcs->len; i++)
        printf(" %s,",((Function *)(funcs->data[i]))->name); 
    printf("\n");
    for (int i = 0; i < funcs->len; i++){
        f = (Function *)(funcs->data[i]);
        printf("%s:\n",f->name);

        // function prologue
        printf("    push rbp\n");
        printf("    mov rbp, rsp\n");
        // Reserve a space for local variables
        int var_num = f->idents->keys->len;
        int heap = 16-((var_num*8)%16)+var_num*8;
        printf("    sub rsp, %d\n", heap);
        // copy every arg into the local variable
        char * reg[] = {"rdi","rsi","rdx","rcx","r8","r9"};
        for (int i = 0; i <= f->args->len-1; i++){
            printf("    mov rax, rbp\n");
            printf("    sub rax, %d\n",(f->args->len-i)*8);
            printf("    mov [rax], %s\n",reg[i]);
        }
        // generate a code from the head
        for (int j = 0; f->code[j]; j++){
            gen_main(f->code[j]);
        }
    }
}
