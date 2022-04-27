#include "stdlib.h"
#include "stdio.h"
#include <sys/mman.h>
#include "translator.h"
#include "reader.h"
#include "DSLtrans.h"

// constant for myOwn binary commands
const int IS_REG = 1 << 5;  // if using regs
const int IS_RAM = 1 << 6;  // if using ram


static int makePushPop     (char *src, Bin_code *dst, int is_push);
static int makePuPRAMReg   (char *src, Bin_code *dst, int is_push);
static int makePuPRAMReg2  (char *src, Bin_code *dst, int is_push);
static int makePushRAMNum  (char *src, Bin_code *dst, int is_push);
static int makePuPRAMRegNum(char *src, Bin_code *dst, int is_push, int is_num_frst);

static int writeNumber(Bin_code *dst, unsigned num);


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
    
    char *buff = dst->buffer;
    for (int i = 0; i < buff_length; ++i)
    {
        buff[i] = 0xC3;
    }
    
    //mprotect(dst->buffer, buff_length, PROT_EXEC);

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


static int makePushPop(char *src, Bin_code *dst, int is_push)
{
    is_debug(if (!src || !dst)  ERR(INVALID_PTR))

    int curr_symb = 0;

    if ((src[curr_symb] & IS_REG) && (src[curr_symb] & IS_RAM))   //for PUSH[reg + num], PUSH[reg + reg], PUSH[reg]
    {
        curr_symb++;

        switch (src[curr_symb++])
        {
            case (2 | IS_REG):          // for PUSH[reg + num]
                makePuPRAMRegNum(src + curr_symb, dst, is_push, 0);
                curr_symb += 2 * sizeof(int) + sizeof(char);
                break;

            case (2 | IS_RAM | IS_REG): // for PUSH[reg + reg]
                makePuPRAMReg2(src + curr_symb, dst, is_push);
                curr_symb += 2 * sizeof(int) + sizeof(char);
                break;

            case (1 | IS_REG):          // for PUSH[reg]
                makePuPRAMReg(src + curr_symb, dst, is_push);
                curr_symb += sizeof(int);
                break;

            case (2 | IS_RAM):          // for PUSH[num + reg]
                makePuPRAMRegNum(src + curr_symb, dst, is_push, 1);
                curr_symb += 2 * sizeof(int) + sizeof(char);
                break;

            case 2:                     // for PUSH[num + num]
                makePushRAMNum(src + curr_symb, dst, is_push);
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
                if (is_push)
                {
                FILL1BYTE(0x50);
                fprintf(dst->asm_version, "PUSH rax\n");
                fflush(dst->asm_version);                         

                }    
                else
                {
                    FILL1BYTE(0x58);
                    fprintf(dst->asm_version, "PUSH rax\n");
                    fflush(dst->asm_version);
                }            
                break;
            case 1:
                if (is_push)
                {
                    FILL1BYTE(0x53);
                    fprintf(dst->asm_version, "PUSH rbx\n");
                    fflush(dst->asm_version);
                }    
                else
                {
                    FILL1BYTE(0x5B);
                    fprintf(dst->asm_version, "POP rbx\n");
                    fflush(dst->asm_version);
                }            
                break;
            case 2:
                if (is_push)    
                {
                    FILL1BYTE(0x51)
                    fprintf(dst->asm_version, "PUSH rcx\n");
                    fflush(dst->asm_version);
                }
                else
                {
                    FILL1BYTE(0x59);
                    fprintf(dst->asm_version, "POP rcx\n");
                    fflush(dst->asm_version);
                }            
                break;

            case 3:
                if (is_push)    
                {
                    FILL1BYTE(0x52);
                    fprintf(dst->asm_version, "PUSH rdx\n");
                    fflush(dst->asm_version);
                }
                else
                {
                    FILL1BYTE(0x5A)
                    fprintf(dst->asm_version, "POP rdx\n");
                    fflush(dst->asm_version);
                }
                break;
        } 
    }
    else if (src[curr_symb] & IS_REG)
    {
        curr_symb++;

        unsigned arg = *((unsigned *) src + curr_symb);
        curr_symb   += sizeof(unsigned);

        MOV_R13_NUMBER(dst, arg);
        MOV_R15_R13RAM;
        PUSH_R15;

    }
    else        // for PUSH num
    {
        curr_symb++;

        if (is_push)
        {
            int arg    = *((int *) (src + curr_symb));
            curr_symb += sizeof(int);

            FILL1BYTE(0x68);     

            writeNumber(dst, arg);

            fprintf(dst->asm_version, "push %d\n", arg);
            PRINT_LINE;
            fflush(dst->asm_version);
        }
        else
        {
            POP_R15;
        }   
    }

    return curr_symb;
}


static int makePuPRAMReg(char *src, Bin_code *dst, int is_push)
{
    is_debug(if (!src || !dst)  ERR(INVALID_PTR));

    int arg   = *((int *) src);

    MOV_R13_REG(arg);   // now we have r*x value in r13
    
    if (is_push)
    {
        MOV_R15_R13RAM;
        PUSH_R15;

        return 0;
    }

    POP_R15;
    MOV_R13RAM_R15;

    return 0;
}


static int makePuPRAMReg2(char *src, Bin_code *dst, int is_push)
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
    if (is_push)
    {
        MOV_R15_R13RAM;
        PUSH_R15;

        return 0;
    }

    POP_R15;
    MOV_R13RAM_R15;

    return 0;
}


static int makePushRAMNum(char *src, Bin_code *dst, int is_push)
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

    MOV_R13_NUMBER(dst, res);   // now we have addr in r13

    if (is_push)
    {
        MOV_R15_R13RAM;
        PUSH_R15;

        return 0;
    }

    POP_R15;
    MOV_R13RAM_R15;

    return 0;
}


static int makePuPRAMRegNum(char *src, Bin_code *dst, int is_push, int is_num_frst)
{
    is_debug(if (!src || !dst)  ERR(INVALID_PTR))

    int num  = 0,
        oper = 0,
        reg  = 0; 

    // for construction PUSH/POP[num + reg]
    num  = *((int *) src);
    oper = *(src + sizeof(int));
    reg  = *((int *) (src + sizeof(int) + sizeof(char)));

    if (!is_num_frst)
    {
        int temp = num;
        num      = reg;
        reg      = temp;
    }

    // now we can do only PUSH/POP[reg + num], not PUSH/POP[num + reg]
    MOV_R13_REG(reg);    

    // now mov r15, num
    writeNumber(dst, num);
    FILL1BYTE(0xBF);
    FILL1BYTE(0x41);

    CALC_R13_R15(oper);

    // now we have correct addr in R13

    if (is_push)
    {
        MOV_R15_R13RAM; // now we have value in r15
        PUSH_R15;
        return 0;
    }

    POP_R15;        // now we have number from stack in r15 & addr i r13
    MOV_R13RAM_R15;

    return 0;
}


static int writeNumber(Bin_code *dst, unsigned num)
{
    is_debug(if (!dst)  ERR(INVALID_PTR));

    // #define CREATE2DIGITS(number)               \
    // dst->buffer[dst->length++] = num % number;  \
    // num /= number;  
    // 0x123456;
    // CREATE2DIGITS(0x100000);
    // CREATE2DIGITS(0x1000);
    // CREATE2DIGITS(0x100);
    // CREATE2DIGITS(0x1);
    
    // #undef CREATE2DIGITS


    for (int i = 0; i < 4; ++i)
    {
        dst->buffer[dst->length++] = num % 0x100;
        num /= 0x100;
    }

    return 0;
}
