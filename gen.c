#include "icc.h"

static int gen_main(Node *node);
static Function *f;

static void gen_lval(Node *node){
    if (node->ty != ND_IDENT && node->ty != ND_REF ) {
        fprintf(stderr, "lhs is not a variable\n");
        exit(1);
    }
    Node *n = node->ty == ND_IDENT ? node : node->lhs;
    Var *v = (Var *)(map_get(f->idents, n->name));
    int offset = v->offset * 8 + 8;
    printf("    mov rax, rbp\n");
    printf("    sub rax, %d\n", offset);
    printf("    push %s\n", 
        node->ty == ND_IDENT ? "rax" : "[rax]");
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
static void gen_call(Node *node){
    char * reg[] = {"rdi","rsi","rdx","rcx","r8","r9"};
    int n = node->args->len-1;
    for (int i = n; i >= 0; i--){
        gen_main((Node *)(node->args->data[i]));
        printf("    pop %s\n",reg[i]);
    }
    printf("    call %s\n", node->name);
    printf("    push rax\n");
}

// binary operation
static void gen_bin(Node *n){
    Node *n_ptr;
    // swap if necessary, for PTR + INT
    // operation
    if ((n->ty == '+' || n->ty == '-') && 
            (n->lhs->eval->ty == PTR)) {
        gen_main(n->rhs);
        gen_main(n->lhs);
        n_ptr = n->lhs;
    } else {
        gen_main(n->lhs);
        gen_main(n->rhs);
        n_ptr = n->rhs;
    }
        printf("    pop rdi\n");
        printf("    pop rax\n");

    char *ir;
    switch (n->ty){
        case ND_EQ:
        case ND_EQN:
            ir = n->ty == ND_EQN ? "setne" : "sete";
            printf("    cmp rdi, rax\n");
            printf("    setne al\n");
            printf("    %s al\n", ir);
            printf("    movzb rax, al\n");
            break;
        case '>':
        case ND_GE:
            ir = n->ty == '>' ? "setl" : "setle";
            printf("    cmp rdi, rax\n");
            printf("    %s al\n",ir);
            printf("    movzb rax, al\n");
            break;
        case '<':
        case ND_LE:
            ir = n->ty == '<' ? "setl" : "setle";
            printf("    cmp rax, rdi\n");
            printf("    %s al\n",ir);
            printf("    movzb rax, al\n");
            break;
        case '+':
        case '-':
            ir = n->ty == '+' ? "add" : "sub";
            int size = 1;
            if (n_ptr->eval->ty == PTR) {
                if (n_ptr->eval->ptrof->ty == PTR)
                    size = 8;   // PTR to PTR
                else if (n_ptr->eval->ptrof->ty == INT)
                    size = 4;   // PTR to INT
                printf("    mov rsi, %d\n", size);
                printf("    mul rsi\n");
                if (n->lhs->eval->ty == PTR) {
                    printf("    %s rdi, rax\n", ir);
                    printf("    mov rax, rdi\n");
                } else
                    printf("    %s rax, rdi\n", ir);
            } else
                printf("    %s rax, rdi\n", ir);
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

static int gen_main(Node *node){
    if (node == (Node *)NULL)
        return 0;
    else if (node->ty == ND_IF) {
        gen_if(node);
        return 0;
    } else if (node->ty == ND_WHILE) {
        gen_while(node);
        return 0;
    } else if (node->ty == ND_FOR) {
        gen_for(node);
        return 0;
    } else if (node->ty == ND_REF){
        gen_lval(node->lhs);
        printf("    pop rax\n");
        printf("    mov rax, [rax]\n");
        printf("    push rax\n");
        printf("    pop rax\n");
        printf("    mov rax, [rax]\n");
        printf("    push rax\n");
    } else if (node->ty == ND_ADDR){
        gen_lval(node->lhs);
    } else if (node->ty == ND_RETURN){
        gen_main(node->lhs);
        printf("    pop rax\n");
        printf("    mov rsp, rbp\n");
        printf("    pop rbp\n");
        printf("    ret\n");
        return 0;
    } else if (node->ty == ND_NUM){
        printf("    push %d\n", node->val);
    } else if (node->ty == ND_IDENT){
        gen_lval(node);
        printf("    pop rax\n");
        printf("    mov rax, [rax]\n");
        printf("    push rax\n");
    } else if (node->ty == ND_CALL){
        gen_call(node);
    } else if (node->ty == '='){
        gen_lval(node->lhs);
        gen_main(node->rhs);
        printf("    pop rdi\n");
        printf("    pop rax\n");
        printf("    mov [rax], rdi\n");
        printf("    push rdi\n");
    } else if (node->ty == ND_NOP) {
        gen_main(node->lhs);
        gen_main(node->rhs);
        return 0;
    } else if (node->ty == ND_COMP) {
        gen_main(node->lhs);
        gen_main(node->rhs);
        printf("    pop rax\n");
        return 0;
    } else { // binary operation or nop Node
        gen_bin(node);
    }
    return 1;
}

static int var_space(Function *f) {
    int var_num = f->idents->keys->len;
    int pad = 16-((var_num*8)%16);
    pad = pad == 16 ? 0 : pad;
    int size = pad+var_num*8;
    return size;
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
        printf("    sub rsp, %d\n", var_space(f));
        // copy every arg into the local variable
        char * reg[] = {"rdi","rsi","rdx","rcx","r8","r9"};
        int n = f->args->len-1;
        for (int i = 0; i <= n; i++){
            printf("    mov rax, rbp\n");
            printf("    sub rax, %d\n",(f->args->len-i)*8);
            printf("    mov [rax], %s\n",reg[n-i]);
        }
        // generate a code from the head
        for (int j = 0; f->code[j]; j++){
            gen_main(f->code[j]);
        }
    }
}
