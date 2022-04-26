#define FILL1BYTE(number)   dst->buffer[dst->length++] = number;

#define PUSH_R13      \
{                     \
    FILL1BYTE(0x55);  \
    FILL1BYTE(0x41);  \
}

#define POP_R13       \
{                     \
    FILL1BYTE(0x5D);  \
    FILL1BYTE(0x41);  \
}

#define POP_R15       \
{                     \
    FILL1BYTE(0x5F);  \
    FILL1BYTE(0x41);  \
}

#define PUSH_R15      \
{                     \
    FILL1BYTE(0x57);  \
    FILL1BYTE(0x41);  \
}

#define MOV_R15_R13     \
{                       \
    FILL1BYTE(0xEF);    \
    FILL1BYTE(0x89);    \
    FILL1BYTE(0x4D);    \
}

#define MOV_R13_R15     \
{                       \
    FILL1BYTE(0xFD);    \
    FILL1BYTE(0x89);    \
    FILL1BYTE(0x4D);    \
}

#define MOV_R13_R_X(first_num)                                              \
{                                                                           \
    FILL1BYTE(first_num);  /* this is mov r13, r*x */     \
    FILL1BYTE(0x89);                                      \
    FILL1BYTE(0x49);                                      \
}

#define MOV_R13_RAX MOV_R13_R_X(0xC5)
#define MOV_R13_RBX MOV_R13_R_X(0xDD)
#define MOV_R13_RCX MOV_R13_R_X(0xCD)
#define MOV_R13_RDX MOV_R13_R_X(0xD5)

// this is macro for mov r13, r*x
#define MOV_R13_REG(reg_num)\
{                               \
    switch (reg_num)            \
    {                           \
    case 0:                     \
        MOV_R13_RAX;            \
                                \
        break;                  \
    case 1:                     \
        MOV_R13_RBX;            \
                                \
        break;                  \
    case 2:                     \
        MOV_R13_RCX;            \
                                \
        break;                  \
    case 3:                     \
        MOV_R13_RDX;            \
                                \
        break;                  \
    default:                    \
        fprintf(stderr, "ERROR OF CHOOSING REG in [%s:%d]\n", __func__, __LINE__); \
        exit(1);                \
    }                           \
}

#define CALC_R13_R15(oper)      \
{                               \
    switch (oper)               \
    {                           \
    case '+':                   \
        FILL1BYTE(0xFD);        \
        FILL1BYTE(0x01);        \
        FILL1BYTE(0x4D);        \
        break;                  \
                                \
    case '-':                   \
        FILL1BYTE(0xFD);        \
        FILL1BYTE(0x29);        \
        FILL1BYTE(0x4D);        \
        break;                  \
                                \
    case '*':                   \
        fprintf(stderr, "That operation doesn't exitst yet\n"); \
        PRINT_LINE;             \
        exit(1);                \
        break;                  \
                                \
    case '/':                   \
        fprintf(stderr, "That operation doesn't exitst yet\n"); \
        PRINT_LINE;             \
        exit(1);                \
        break;                  \
                                \
    default:                    \
        fprintf(stderr, "Unknown sign of arif oper %c in [%s:%d]\n", oper, __func__, __LINE__); \
        exit(1);                \
        break;                  \
    }                           \
}

#define MOV_R15_R13RAM          \
{/* mov r15, [r13] */           \
    FILL1BYTE(0x00);            \
    FILL1BYTE(0x7D);            \
    FILL1BYTE(0x8B);            \
    FILL1BYTE(0x4D);            \
}

#define MOV_R13_NUMBER(dst, num)    \
{/* mov r13, number*/               \
    writeNumber(dst, num);          \
    FILL1BYTE(0x25);                \
    FILL1BYTE(0x2C);                \
    FILL1BYTE(0x8B);                \
    FILL1BYTE(0x4C);                \
}

#define MOV_R13RAM_R15              \
{/* mov [r13], r15*/                \
    FILL1BYTE(0x00);                \
    FILL1BYTE(0x7D);                \
    FILL1BYTE(0x89);                \
    FILL1BYTE(0x4D);                \
}