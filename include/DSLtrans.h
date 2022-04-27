#define FILL1BYTE(number)   dst->buffer[dst->length++] = number;

#define PUSH_R13                                            \
{                                                           \
    FILL1BYTE(0x41);                                        \
    FILL1BYTE(0x55);                                        \
    is_debug(fprintf(dst->asm_version, "push r13\n");       \
             fflush(dst->asm_version));                     \
}

#define POP_R13                                             \
{                                                           \
    FILL1BYTE(0x41);                                        \
    FILL1BYTE(0x5D);                                        \
    is_debug(fprintf(dst->asm_version, "pop r13\n");        \
             fflush(dst->asm_version));                     \
}

#define POP_R15                                             \
{                                                           \
    FILL1BYTE(0x41);                                        \
    FILL1BYTE(0x5F);                                        \
    is_debug(fprintf(dst->asm_version, "pop r15\n");        \
             fflush(dst->asm_version));                     \
}

#define PUSH_R15                                            \
{                                                           \
    FILL1BYTE(0x41);                                        \
    FILL1BYTE(0x57);                                        \
    is_debug(fprintf(dst->asm_version, "push r15\n");       \
             fflush(dst->asm_version));                     \
}

#define MOV_R15_R13                                         \
{                                                           \
    FILL1BYTE(0x4D);                                        \
    FILL1BYTE(0x89);                                        \
    FILL1BYTE(0xEF);                                        \
    is_debug(fprintf(dst->asm_version, "mov r15, r13\n");   \
             fflush(dst->asm_version));                     \
}

#define MOV_R13_R15                                         \
{                                                           \
    FILL1BYTE(0x4D);                                        \
    FILL1BYTE(0x89);                                        \
    FILL1BYTE(0xFD);                                        \
    is_debug(fprintf(dst->asm_version, "mov r13, r15\n");   \
             fflush(dst->asm_version));                     \
}

#define MOV_R13_R_X(first_num)                              \
{                                                           \
    FILL1BYTE(0x49);                                        \
    FILL1BYTE(0x89);                                        \
    FILL1BYTE(first_num);  /* this is mov r13, r*x */       \
}

#define MOV_R13_RAX MOV_R13_R_X(0xC5)
#define MOV_R13_RBX MOV_R13_R_X(0xDD)
#define MOV_R13_RCX MOV_R13_R_X(0xCD)
#define MOV_R13_RDX MOV_R13_R_X(0xD5)

// this is macro for mov r13, r*x
#define MOV_R13_REG(reg_num)                                                        \
{                                                                                   \
    switch (reg_num)                                                                \
    {                                                                               \
    case 0:                                                                         \
        MOV_R13_RAX;                                                                \
        is_debug(fprintf(dst->asm_version, "mov r13, rax\n");                       \
                 fflush(dst->asm_version));                                         \
                                                                                    \
        break;                                                                      \
    case 1:                                                                         \
        MOV_R13_RBX;                                                                \
        is_debug(fprintf(dst->asm_version, "mov r13, rbx\n");                       \
                 fflush(dst->asm_version));                                         \
                                                                                    \
        break;                                                                      \
    case 2:                                                                         \
        MOV_R13_RCX;                                                                \
        is_debug(fprintf(dst->asm_version, "mov r13, rcx\n");                       \
                 fflush(dst->asm_version));                                         \
                                                                                    \
        break;                                                                      \
    case 3:                                                                         \
        MOV_R13_RDX;                                                                \
        is_debug(fprintf(dst->asm_version, "mov r13, rdx\n");                       \
                 fflush(dst->asm_version));                                         \
                                                                                    \
        break;                                                                      \
    default:                                                                        \
        fprintf(stderr, "ERROR OF CHOOSING REG in [%s:%d]\n", __func__, __LINE__);  \
        exit(1);                                                                    \
    }                                                                               \
}

#define CALC_R13_R15(oper)                                                                      \
{                                                                                               \
    switch (oper)                                                                               \
    {                                                                                           \
    case '+':                                                                                   \
        FILL1BYTE(0x4D);                                                                        \
        FILL1BYTE(0x01);                                                                        \
        FILL1BYTE(0xFD);                                                                        \
        is_debug(fprintf(dst->asm_version, "add r13, r15\n");                                   \
                 fflush(dst->asm_version));                                                     \
        break;                                                                                  \
                                                                                                \
    case '-':                                                                                   \
        FILL1BYTE(0x4D);                                                                        \
        FILL1BYTE(0x29);                                                                        \
        FILL1BYTE(0xFD);                                                                        \
        is_debug(fprintf(dst->asm_version, "sub r13, r15\n");                                   \
                 fflush(dst->asm_version));                                                     \
        break;                                                                                  \
                                                                                                \
    case '*':                                                                                   \
        fprintf(stderr, "That operation doesn't exitst yet\n");                                 \
        PRINT_LINE;                                                                             \
        exit(1);                                                                                \
        break;                                                                                  \
                                                                                                \
    case '/':                                                                                   \
        fprintf(stderr, "That operation doesn't exitst yet\n");                                 \
        PRINT_LINE;                                                                             \
        exit(1);                                                                                \
        break;                                                                                  \
                                                                                                \
    default:                                                                                    \
        fprintf(stderr, "Unknown sign of arif oper %c in [%s:%d]\n", oper, __func__, __LINE__); \
        exit(1);                                                                                \
        break;                                                                                  \
    }                                                                                           \
}

#define MOV_R15_R13RAM                                          \
{/* mov r15, [r13] */                                           \
    FILL1BYTE(0x4D);                                            \
    FILL1BYTE(0x8B);                                            \
    FILL1BYTE(0x7D);                                            \
    FILL1BYTE(0x00);                                            \
    is_debug(fprintf(dst->asm_version, "mov r15, [r13]\n");     \
             fflush(dst->asm_version));                         \
}

#define MOV_R13_NUMBER(dst, num)                                \
{/* mov r13, number*/                                           \
    writeNumber(dst, num);                                      \
    FILL1BYTE(0x4C);                                            \
    FILL1BYTE(0x8B);                                            \
    FILL1BYTE(0x2C);                                            \
    FILL1BYTE(0x25);                                            \
    is_debug(fprintf(dst->asm_version, "mov r13, %d\n", num);   \
             fflush(dst->asm_version));                         \
}

#define MOV_R13RAM_R15                                          \
{/* mov [r13], r15*/                                            \
    FILL1BYTE(0x4D);                                            \
    FILL1BYTE(0x89);                                            \
    FILL1BYTE(0x7D);                                            \
    FILL1BYTE(0x00);                                            \
    is_debug(fprintf(dst->asm_version, "mov [r13], r15\n");     \
             fflush(dst->asm_version));                         \
}