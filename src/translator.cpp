#include "stdlib.h"
#include "stdio.h"
#include "translator.h"
#include "reader.h"

// constant for myOwn binary commands
const int IS_REG = 1 << 5;  // if using regs
const int IS_RAM = 1 << 6;  // if using ram


#define PUSH_R13                        \
{                                       \
    dst->buffer[dst->length++] = 0x55;  \
    dst->buffer[dst->length++] = 0x41;  \
}

#define MOV_R13_REG(first_num)                                            \
{                                                                           \
    dst->buffer[dst->length++] = first_num;  /* this is mov r13, [r*x] */   \
    dst->buffer[dst->length++] = 0x8B;  /* this is mov r13, [r*x] */        \
    dst->buffer[dst->length++] = 0x4C;  /* this is mov r13, [r*x] */        \
}

#define MOV_R13_RAX MOV_R13_REG(0x28);
#define MOV_R13_RBX MOV_R13_REG(0x2B);
#define MOV_R13_RCX MOV_R13_REG(0x29);
#define MOV_R13_RDX MOV_R13_REG(0x2A);

#define FILL1BYTE(number)   dst->buffer[dst->length++] = number;

// int SourceCtor(Sourse_code *src)
// {
//     is_debug(if (!src)  ERR(INVALID_PTR))

//     return 0;
// }

int SourceDtor(Sourse_code *src)
{
    is_debug(if (!src)  ERR(INVALID_PTR))

    free(src->buffer);

    return 0;
}


int BinCtor(Bin_code *dst, long buff_length)
{
    is_debug(if (!dst)  ERR(INVALID_PTR))

    dst->buffer      = (char *) calloc(buff_length, sizeof(char));
    dst->asm_version = fopen("ASM_LOGS", "w");
    dst->capacity    = buff_length;

    return 0;
}


int BinDtor(Bin_code *dst)
{
    is_debug(if (!dst)  ERR(INVALID_PTR))

    free(dst->buffer);
    fclose(dst->asm_version);

    return 0;
}


int translation(Sourse_code *src, Bin_code *dst)
{
    is_debug(if (!src || !dst)  ERR(INVALID_PTR))

    char *src_arr = src->buffer;

    for (long i = 0; i < src->length; ++i)
    {
        switch (src_arr[i] & (IS_REG - 1))  // for getting only cmd code (without any info about regs and ram)
        {
            #define DEF_CMD(number, name, DSLcode)          \
                case number:                                \
                    DSLcode;                                \
                    break;

            #include "commands.inc"
            #undef DEF_CMD
        }
    }

    return 0;
}


int makePush(char *src, Bin_code *dst)
{
    is_debug(if (!src || !dst)  ERR(INVALID_PTR))

    int curr_symb = 0;

    if ((src[curr_symb] & IS_REG) && (src[curr_symb] & IS_RAM))   //for PUSH[reg + num], PUSH[reg + reg], PUSH[reg]
    {
        curr_symb++;

        switch (src[curr_symb++])
        {
            case (2 | IS_REG):          // for PUSH[reg + num]
                makePushRAMRegNum(src + curr_symb, dst);
                curr_symb += 2 * sizeof(int) + sizeof(char);
                break;

            case (2 | IS_RAM | IS_REG): // for PUSH[reg + reg]
                break;

            case (1 | IS_REG):          // for PUSH[reg]
                makePushRAMReg(src + curr_symb, dst);
                curr_symb += sizeof(int);
                break;

            case (2 | IS_RAM):          // for PUSH[num + reg]
                break;

            case 2:                     // for PUSH[num + num]
                makePushRAMNum(src + curr_symb, dst);
                curr_symb += 2 * sizeof(int) + sizeof(char);
                break;

            default:
                fprintf(stderr, "!!!!!INVALID MODE OF PUSH!!!!!!\n");
                PRINT_LINE;
                exit(1);
        }
    }
    else if (src[curr_symb] & IS_REG)   // for PUSH reg
    {
        curr_symb++;

        int arg    = *((int *) (src + curr_symb));
        curr_symb += sizeof(int);

        switch (arg)
        {
            case 0:
                dst->buffer[dst->length++] = 0x50;
                break;
            case 1:
                dst->buffer[dst->length++] = 0x53;
                break;
            case 2:
                dst->buffer[dst->length++] = 0x51;
                break;
            case 3:
                dst->buffer[dst->length++] = 0x52;
                break;
        } 
    }
    else        // for PUSH num
    {
        curr_symb++;

        int arg    = *((int *) (src + curr_symb));
        curr_symb += sizeof(int);

        #define CREATE2DIGITS(number)               \
        dst->buffer[dst->length++] = arg / number;  \
        arg %= number;  

        CREATE2DIGITS(0x100000);
        CREATE2DIGITS(0x1000);
        CREATE2DIGITS(0x10);
        CREATE2DIGITS(0x1);

        dst->buffer[dst->length++] = 0x68;
        
        #undef CREATE2DIGITS
    }

    return curr_symb;
}


int makePushRAMReg(char *src, Bin_code *dst)
{
    is_debug(if (!src || !dst)  ERR(INVALID_PTR));

    int arg   = *((int *) src);

    switch (arg)
    {
    case 0: //PUSH[rax]
        MOV_R13_RAX;
        PUSH_R13;

        break;
    case 1:
        MOV_R13_RBX;
        PUSH_R13;

        break;
    case 2:
        MOV_R13_RCX;
        PUSH_R13;

        break;
    case 3:
        MOV_R13_RDX;
        PUSH_R13;

        break;
    default:
        fprintf(stderr, "ERROR OF CHOOSING REG in [%s:%d]\n", __func__, __LINE__);
        exit(1);
    }

    return 0;
}


int makePushRAMNum(char *src, Bin_code *dst)
{
    is_debug(if (!src || !dst)  ERR(INVALID_PTR));

    int first  = *((int *) src),
        oper   = *(src + sizeof(int)),
        second = *((int *) (src + sizeof(int) + sizeof(char))),
        res    = 0;

    switch (oper)
    {
        case '+':
            res = first + second;
            break;
        case '-':
            res = first - second;
            break;
        case '*':
            res = first * second;
            break;
        case '/':
            res = first / second;
            break;
    }

    #define CREATE2DIGITS(number)               \
    dst->buffer[dst->length++] = res / number;  \
    res %= number;  

    CREATE2DIGITS(0x100000);
    CREATE2DIGITS(0x1000);
    CREATE2DIGITS(0x10);
    CREATE2DIGITS(0x1);
    
    #undef CREATE2DIGITS

    FILL1BYTE(0x25);
    FILL1BYTE(0x2C);
    FILL1BYTE(0x8B);
    FILL1BYTE(0x4C);

    PUSH_R13;

    return 0;
}


int makePushRAMRegNum(char *src, Bin_code *dst, int is_num_frst)
{
    is_debug(if (!src || !dst)  ERR(INVALID_PTR))

    if 

    return 0;
}
