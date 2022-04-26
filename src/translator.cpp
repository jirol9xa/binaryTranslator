#include "stdlib.h"
#include "stdio.h"
#include "translator.h"
#include "reader.h"
#include "DSLtrans.h"

// constant for myOwn binary commands
const int IS_REG = 1 << 5;  // if using regs
const int IS_RAM = 1 << 6;  // if using ram

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


int makePop(char *src, Bin_code *dst)
{
    is_debug(if (!src || !dst)  ERR(INVALID_PTR));

    int curr_symb = 0;

    if ((src[curr_symb] & IS_REG) && (src[curr_symb] & IS_RAM))     // for POP[reg + num], POP[reg + reg], POP[reg]


    return curr_symb;
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
                makePushRAMRegNum(src + curr_symb, dst, 0);
                curr_symb += 2 * sizeof(int) + sizeof(char);
                break;

            case (2 | IS_RAM | IS_REG): // for PUSH[reg + reg]
                break;

            case (1 | IS_REG):          // for PUSH[reg]
                makePushRAMReg(src + curr_symb, dst);
                curr_symb += sizeof(int);
                break;

            case (2 | IS_RAM):          // for PUSH[num + reg]
                makePushRAMRegNum(src + curr_symb, dst, 1);
                curr_symb += 2 * sizeof(int) + sizeof(char);
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

        writeNumber(dst, arg);

        FILL1BYTE(0x68);        
    }

    return curr_symb;
}


int makePushRAMReg(char *src, Bin_code *dst)
{
    is_debug(if (!src || !dst)  ERR(INVALID_PTR));

    int arg   = *((int *) src);

    MOV_R13_REG_RAM(arg);
    PUSH_R13;

    return 0;
}


int makePushRAMReg2(char *src, Bin_code *dst)
{
    is_debug(if (!src || !dst)  ERR(INVALID_PTR));

    int reg1 = *((int *) src),
        oper = *(src + sizeof(int)),
        reg2 = *((int *) (src + sizeof(int) + sizeof(char)));
    
    MOV_R13_REG(reg1);
    MOV_R15_R13;
    MOV_R13_REG(reg2);

    CALC_R13_R15(oper);

    // now we have correct addr in R13

    MOV_R15_R13RAM;

    PUSH_R15;

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

    MOV_R13_NUMBER(dst, res);

    PUSH_R13;

    return 0;
}


int makePushRAMRegNum(char *src, Bin_code *dst, int is_num_frst)
{
    is_debug(if (!src || !dst)  ERR(INVALID_PTR))

    int num  = 0,
        oper = 0,
        reg  = 0; 

    // for construction PUSH[num + reg]
    num  = *((int *) src);
    oper = *(src + sizeof(int));
    reg  = *((int *) (src + sizeof(int) + sizeof(char)));

    if (!is_num_frst)
    {
        int temp = num;
        num      = reg;
        reg      = temp;
    }

    // now we can do only PUSH[reg + num], not PUSH[num + reg]
    MOV_R13_REG(reg);    

    // now mov r15, num
    writeNumber(dst, num);
    FILL1BYTE(0xBF);
    FILL1BYTE(0x41);

    CALC_R13_R15(oper);

    // now we have correct addr in R13

    MOV_R15_R13RAM;

    PUSH_R15;

    return 0;
}


int writeNumber(Bin_code *dst, int num)
{
    is_debug(if (!dst)  ERR(INVALID_PTR));

    #define CREATE2DIGITS(number)               \
    dst->buffer[dst->length++] = num / number;  \
    num %= number;  

    CREATE2DIGITS(0x100000);
    CREATE2DIGITS(0x1000);
    CREATE2DIGITS(0x10);
    CREATE2DIGITS(0x1);
    
    #undef CREATE2DIGITS

    return 0;
}
