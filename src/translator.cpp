#include "stdlib.h"
#include "stdio.h"
#include <sys/mman.h>
#include "errno.h"
#include "string.h"
#include "translator.h"
#include "reader.h"
#include "DSLtrans.h"
#include "limits.h"

#ifndef PAGESIZE
    #define PAGESIZE 4096
#endif


// constant for myOwn binary commands
const int IS_REG = 1 << 5;  // if using regs
const int IS_RAM = 1 << 6;  // if using ram

static int saveAllRegs(Bin_code *dst);
static int restoreAllRegs(Bin_code *dst);

static int makePushPop     (unsigned char *src, Bin_code *dst, int is_push);
static int makePuPRAMReg   (unsigned char *src, Bin_code *dst, int is_push);
static int makePuPRAMReg2  (unsigned char *src, Bin_code *dst, int is_push);
static int makePuPRAMNum   (unsigned char *src, Bin_code *dst, int is_push);
static int makePuPRAMRegNum(unsigned char *src, Bin_code *dst, int is_push,  int is_num_frst);
static int makeJmp         (unsigned char *src, Bin_code *dst, int jmp_code, int curr_symb);
static int makeCall        (unsigned char *src, Bin_code *dst);
static int makeArifm       (Bin_code *dst, int oper);
static int makeOut         (Bin_code *dst);
static int makeIn          (Bin_code *dst);

static int writeNumber(Bin_code *dst, u_int64_t num, int size = 4);

static void wrapPrintf(int  arg);
static int  wrapScanf ();

static int labelPushBack(Bin_code *dst, long src_ip);
static int getLabels    (Bin_code *dst, unsigned char *src_arr);

int LabComp(const void *first, const void *second);

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

    void *temp_ptr = nullptr;

    if (posix_memalign((void **) &(temp_ptr), PAGESIZE, buff_length * 16 + 64))
    {
        PRINT_LINE;
        fprintf(stderr, "!!! Posix error !!!\n");
        exit(errno);
    }

    if (!temp_ptr)  ERR(MEM_OVERFLOW);

    dst->buffer = (unsigned char *) temp_ptr;

    unsigned char *buff = dst->buffer;
    for (int i = 0; i < buff_length; ++i)
    {
        buff[i] = 0xC3;
    }

    //mprotect(dst->buffer, buff_length, PROT_EXEC);

    dst->asm_version = fopen("ASM_LOGS", "w");
    dst->capacity    = buff_length;

    dst->dst_ip = 0;
    dst->src_ip = 0;


    temp_ptr = (Label *) calloc(buff_length / 5, sizeof(Label));
    if (!temp_ptr)  ERR(MEM_OVERFLOW);

    dst->labels.data     = (Label *) temp_ptr;
    dst->labels.size     = 0;
    dst->labels.capacity = buff_length / 5;

    return 0;
}


int BinDtor(Bin_code *dst)
{
    is_debug(if (!dst)  ERR(INVALID_PTR))

    free(dst->buffer);
    fclose(dst->asm_version);

    free(dst->labels.data);

    return 0;
}


int translation(Sourse_code *src, Bin_code *dst)
{
    is_debug(if (!src || !dst)  ERR(INVALID_PTR))

    unsigned char *src_arr = src->buffer;

    saveAllRegs(dst);

    for (long i = 0; i < src->length; )
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

    restoreAllRegs(dst);

    FILL1BYTE(0xC3); fprintf(dst->asm_version, "ret"); fflush(dst->asm_version);

    Label *lab = dst->labels.data;
    for (int i = 0; i < dst->labels.size; i++)
    {
        fprintf(stderr, "src_ip = %d, dst_ip = %d\n", lab[i].src_ip, lab[i].dst_ip);
    }

    getLabels(dst, src->buffer);

    return 0;
}


static int makePushPop(unsigned char *src, Bin_code *dst, int is_push)
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
                makePuPRAMNum(src + curr_symb, dst, is_push);
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

        unsigned arg = *((unsigned *) (src + curr_symb));
        curr_symb   += sizeof(unsigned);

        switch (arg)
        {
            case 0:
                if (is_push)
                {
                FILL1BYTE(0x50);
                fprintf(dst->asm_version, "push rax\n");
                fflush(dst->asm_version);
                }
                else
                {
                    FILL1BYTE(0x58);
                    fprintf(dst->asm_version, "pop rax\n");
                    fflush(dst->asm_version);
                }
                break;
            case 1:
                if (is_push)
                {
                    FILL1BYTE(0x53);
                    fprintf(dst->asm_version, "push rbx\n");
                    fflush(dst->asm_version);
                }
                else
                {
                    FILL1BYTE(0x5B);
                    fprintf(dst->asm_version, "pop rbx\n");
                    fflush(dst->asm_version);
                }
                break;
            case 2:
                if (is_push)
                {
                    FILL1BYTE(0x51)
                    fprintf(dst->asm_version, "push rcx\n");
                    fflush(dst->asm_version);
                }
                else
                {
                    FILL1BYTE(0x59);
                    fprintf(dst->asm_version, "pop rcx\n");
                    fflush(dst->asm_version);
                }
                break;

            case 3:
                if (is_push)
                {
                    FILL1BYTE(0x52);
                    fprintf(dst->asm_version, "push rdx\n");
                    fflush(dst->asm_version);
                }
                else
                {
                    FILL1BYTE(0x5A)
                    fprintf(dst->asm_version, "pop rdx\n");
                    fflush(dst->asm_version);
                }
                break;
        }
    }
    else if (src[curr_symb] & IS_RAM)
    {
        curr_symb++;

        PRINT_LINE;

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
            unsigned arg = *((unsigned *) (src + curr_symb));
            curr_symb   += sizeof(unsigned);

            FILL1BYTE(0x68);

            writeNumber(dst, arg);

            fprintf(dst->asm_version, "push %d\n", arg);
            fflush(dst->asm_version);
        }
        else
        {
            POP_R15;
        }
    }

    return curr_symb;
}


static int makePuPRAMReg(unsigned char *src, Bin_code *dst, int is_push)
{
    is_debug(if (!src || !dst)  ERR(INVALID_PTR));

    unsigned arg = *((unsigned *) src);

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


static int makePuPRAMReg2(unsigned char *src, Bin_code *dst, int is_push)
{
    is_debug(if (!src || !dst)  ERR(INVALID_PTR));

    unsigned reg1 = *((unsigned *) src);
    char     oper = *(src + sizeof(unsigned));
    unsigned reg2 = *((unsigned *) (src + sizeof(unsigned) + sizeof(char)));

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


static int makePuPRAMNum(unsigned char *src, Bin_code *dst, int is_push)
{
    is_debug(if (!src || !dst)  ERR(INVALID_PTR));

    unsigned first  = *((unsigned *) src);
    char     oper   = *(src + sizeof(unsigned));
    unsigned second = *((unsigned *) (src + sizeof(unsigned) + sizeof(char)));
    unsigned res    = 0;

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


static int makePuPRAMRegNum(unsigned char *src, Bin_code *dst, int is_push, int is_num_frst)
{
    is_debug(if (!src || !dst)  ERR(INVALID_PTR))

    unsigned num  = 0;
    char     oper = 0;
    unsigned reg  = 0;

    // for construction PUSH/POP[num + reg]
    num  = *((unsigned *) src);
    oper = *(src + sizeof(unsigned));
    reg  = *((unsigned *) (src + sizeof(unsigned) + sizeof(char)));

    if (!is_num_frst)
    {
        unsigned temp = num;
        num           = reg;
        reg           = temp;
    }

    // now we can do only PUSH/POP[reg + num], not PUSH/POP[num + reg]
    MOV_R13_REG(reg);

    // now mov r15, num
    FILL1BYTE(0x41);
    FILL1BYTE(0xBF);
    writeNumber(dst, num);
    fprintf(dst->asm_version, "mov r15, %d\n", num);

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


static int writeNumber(Bin_code *dst, u_int64_t num, int size)
{
    is_debug(if (!dst)  ERR(INVALID_PTR));

    for (int i = 0; i < size; ++i)
    {
        dst->buffer[dst->length++] = num % 0x100;
        num /= 0x100;
    }

    return 0;
}


static int saveAllRegs(Bin_code *dst)
{
    is_debug(if (!dst)  ERR(INVALID_PTR));

    FILE *Asm = dst->asm_version;

    FILL1BYTE(0x50); fprintf(Asm, "push rax\n"); fflush(Asm);
    FILL1BYTE(0x53); fprintf(Asm, "push rbx\n"); fflush(Asm);
    FILL1BYTE(0x51); fprintf(Asm, "push rcx\n"); fflush(Asm);
    FILL1BYTE(0x52); fprintf(Asm, "push rdx\n"); fflush(Asm);
    FILL1BYTE(0x56); fprintf(Asm, "push rsi\n"); fflush(Asm);
    FILL1BYTE(0x57); fprintf(Asm, "push rdi\n"); fflush(Asm);
    PUSH_R13;
    PUSH_R15;

    return 0;
}


static int restoreAllRegs(Bin_code *dst)
{
    is_debug(if (!dst)  ERR(INVALID_PTR));

    FILE *Asm = dst->asm_version;

    POP_R15;
    POP_R13;
    FILL1BYTE(0x5F); fprintf(Asm, "pop rdi\n"); fflush(Asm);
    FILL1BYTE(0x5E); fprintf(Asm, "pop rsi\n"); fflush(Asm);
    FILL1BYTE(0x5A); fprintf(Asm, "pop rdx\n"); fflush(Asm);
    FILL1BYTE(0x59); fprintf(Asm, "pop rcx\n"); fflush(Asm);
    FILL1BYTE(0x5B); fprintf(Asm, "pop rbx\n"); fflush(Asm);
    FILL1BYTE(0x58); fprintf(Asm, "pop rax\n"); fflush(Asm);

    return 0;
}


static int makeArifm(Bin_code *dst, int oper)
{
    is_debug(if (!dst)  ERR(INVALID_PTR));

    POP_R15;
    POP_R13;
    CALC_R13_R15(oper);
    PUSH_R13;

    return 0;
}


static int makeOut(Bin_code *dst)
{
    is_debug(if (!dst)  ERR(INVALID_PTR));

    POP_R15;

    PUSH_RAX;

    FILL1BYTE(0x55); fprintf(dst->asm_version, "push rbp\n"); fflush(dst->asm_version);

    FILL1BYTE(0x48);
    FILL1BYTE(0x89);
    FILL1BYTE(0xE5);
    fprintf(dst->asm_version, "mov rbp, rsp\n"); fflush(dst->asm_version);

    FILL1BYTE(0x4C);
    FILL1BYTE(0x89);
    FILL1BYTE(0xFF);
    fprintf(dst->asm_version, "mov rdi, r15\n"); fflush(dst->asm_version);

    FILL1BYTE(0x48);
    FILL1BYTE(0x31);
    FILL1BYTE(0xC0);
    fprintf(dst->asm_version, "xor rax,rax\n"); fflush(dst->asm_version);


    MOV_R13_NUMBER(dst, (u_int64_t) &wrapPrintf);

    FILL1BYTE(0x41);
    FILL1BYTE(0xFF);
    FILL1BYTE(0xD5);
    fprintf(dst->asm_version, "call r13\n"); fflush(dst->asm_version);

    FILL1BYTE(0x5D); fprintf(dst->asm_version, "pop rbp\n"); fflush(dst->asm_version);

    POP_RAX;

    return 0;
}


static int makeIn(Bin_code *dst)
{
    is_debug(if (!dst)  ERR(INVALID_PTR));

    // MOV_R13_RAX;    // saving rax in r15
    // MOV_R15_R13;    // saving rax in r15

    // FILL1BYTE(0x49);    // mov r15, rax
    // FILL1BYTE(0x89);    // mov r15, rax
    // FILL1BYTE(0xC7);    // mov r15, rax

    FILL1BYTE(0x50); fprintf(dst->asm_version, "push rax\n"); fflush(dst->asm_version);
    FILL1BYTE(0x53); fprintf(dst->asm_version, "push rbx\n"); fflush(dst->asm_version);
    FILL1BYTE(0x51); fprintf(dst->asm_version, "push rcx\n"); fflush(dst->asm_version);
    FILL1BYTE(0x52); fprintf(dst->asm_version, "push rdx\n"); fflush(dst->asm_version);

    FILL1BYTE(0x48);
    FILL1BYTE(0x31);
    FILL1BYTE(0xC0);
    fprintf(dst->asm_version, "xor rax,rax\n"); fflush(dst->asm_version);

    MOV_R13_NUMBER(dst, (u_int64_t) &wrapScanf);

    // FILL1BYTE(0x4C);
    // FILL1BYTE(0x89);
    // FILL1BYTE(0xEB);
    // fprintf(dst->asm_version, "mov rax, r13\n"); fflush(dst->asm_version);

    //FILL1BYTE(0x68);
    //writeNumber(dst, 5433);

    FILL1BYTE(0x55); fprintf(dst->asm_version, "push rbp\n"); fflush(dst->asm_version);

    FILL1BYTE(0x48);
    FILL1BYTE(0x89);
    FILL1BYTE(0xE5);
    fprintf(dst->asm_version, "mov rpb, rsp\n"); fflush(dst->asm_version);

    FILL1BYTE(0x41);
    FILL1BYTE(0xFF);
    FILL1BYTE(0xD5);
    fprintf(dst->asm_version, "call r13\n"); fflush(dst->asm_version);

    FILL1BYTE(0x5D); fprintf(dst->asm_version, "pop rbp\n"); fflush(dst->asm_version);

    FILL1BYTE(0x49);
    FILL1BYTE(0x89);
    FILL1BYTE(0xC7);
    fprintf(dst->asm_version, "mov r15, rax\n"); fflush(dst->asm_version);

    FILL1BYTE(0x5A); fprintf(dst->asm_version, "pop rdx\n"); fflush(dst->asm_version);
    FILL1BYTE(0x59); fprintf(dst->asm_version, "pop rcx\n"); fflush(dst->asm_version);
    FILL1BYTE(0x5B); fprintf(dst->asm_version, "pop rbx\n"); fflush(dst->asm_version);
    FILL1BYTE(0x58); fprintf(dst->asm_version, "pop rax\n"); fflush(dst->asm_version);

    PUSH_R15;
    //FILL1BYTE(0x4C);    // resoting rax
    //FILL1BYTE(0x89);    // resoting rax
    //FILL1BYTE(0xF8);    // resoting rax
    //fprintf(dst->asm_version, "mov rax, r15\n"); fflush(dst->asm_version);

    return 0;
}


static void wrapPrintf(int arg)
{
    printf("%d\n", arg);
}


static int wrapWrapScanf()
{
    int value = 0;
    char *format = "%d";

    //printf("format = %p, &value = %p\n", format, &value);

    while (!scanf(format, &value))
    {
        while (getchar() != '\n')   continue;
    }

    return value;
}


static int wrapScanf()
{
    return wrapWrapScanf();
}


static int labelPushBack(Bin_code *dst, long src_ip)
{
    is_debug(if (!dst)  ERR(INVALID_PTR));

    if (dst->labels.size + 1 >= dst->labels.capacity)
    {
        void *temp_ptr = realloc(dst->labels.data, dst->labels.capacity * 2 * sizeof(Label));
        if (!temp_ptr)  ERR(MEM_OVERFLOW);

        dst->labels.data      = (Label *) temp_ptr;
        dst->labels.capacity *= 2;
    }

    dst->labels.data[dst->labels.size].dst_ip = dst->length;
    dst->labels.data[dst->labels.size].src_ip = src_ip;

    dst->labels.size++;

    return 0;
}


static int getLabels(Bin_code *dst, unsigned char *src_arr)
{
    is_debug(if (!src_arr || !dst)  ERR(INVALID_PTR));

    //dst->src_ip = 0;
    ///*dst->*/dst_ip = 10;   // saving regs in stack

    int src_ip       = 0;
    u_int64_t dst_ip = 10;    // saving regs in stack
    int last_offset  = 0; 

    // in this gunc we've already filled array with labels, so
    // we can sort that

    qsort(dst->labels.data, dst->labels.size, sizeof(Label), LabComp);

    // now we have sorted arr, so we need make search 1 time for 'size' segment

    for (int lab_num = 0; lab_num < dst->labels.size; ++lab_num)
    {
        PRINT_LINE;
        for (; src_ip < dst->labels.data[lab_num].src_ip;)
        {
            fprintf(stderr, "src_ip = %d, lab_ip = %d\n", src_ip, dst->labels.data[lab_num].src_ip);
            switch (src_arr[src_ip] & (IS_REG - 1))
            {
                case 0: // IN
                    /*dst->*/src_ip++;
                    /*dst->*/dst_ip += 30;
                    last_offset = 30;
                    break;

                case 1: // HLT
                    src_ip++;
                    dst_ip++;
                    last_offset = 1;
                    break;

                case 2: // PUSH
                    PRINT_LINE;
                    if ((src_arr[/*dst->*/src_ip] & IS_REG) && (src_arr[/*dst->*/src_ip] & IS_RAM))
                    {
                        /*dst->*/src_ip++;

                        switch (src_arr[/*dst->*/src_ip++])
                        {
                            case (2 | IS_REG):
                                /*dst->*/src_ip += 2 * sizeof(int) + sizeof(char);
                                /*dst->*/dst_ip += 18;
                                last_offset = 18;
                                break;

                            case (2 | IS_RAM | IS_REG):
                                /*dst->*/src_ip += 2 * sizeof(int) + sizeof(char);
                                /*dst->*/dst_ip += 18;
                                last_offset = 18;
                                break;

                            case (1 | IS_REG):
                                /*dst->*/src_ip += sizeof(int);
                                /*dst->*/dst_ip += 9;
                                last_offset = 9;
                                break;

                            case (2 | IS_RAM):
                                /*dst->*/src_ip += 2 * sizeof(int) + sizeof(char);
                                /*dst->*/dst_ip += 18;
                                last_offset = 18;
                                break;

                            case (2):
                                /*dst->*/src_ip += 2 * sizeof(int) + sizeof(char);
                                /*dst->*/dst_ip += 12;
                                last_offset = 12;
                                break;
                        }
                    }
                    else if (src_arr[/*dst->*/src_ip] & IS_REG) // for PUSH reg
                    {
                        /*dst->*/src_ip += sizeof(unsigned) + sizeof(char);
                        /*dst->*/dst_ip++;
                        last_offset = 1;
                    }
                    else if (src_arr[/*dst->*/src_ip] & IS_RAM)
                    {
                        /*dst->*/src_ip += sizeof(unsigned) + sizeof(char);
                        /*dst->*/dst_ip += 12;
                        last_offset = 12;
                    }
                    else    // for PUSH num
                    {
                        /*dst->*/src_ip += sizeof(unsigned) + sizeof(char);
                        /*dst->*/dst_ip += 5;
                        last_offset = 5;
                    }
                    break;

                case 3: // OUT
                    /*dst->*/src_ip++;
                    /*dst->*/dst_ip += 21;
                    last_offset = 23;
                    break;

                case 4: // ADD
                    /*dst->*/src_ip += sizeof(char);
                    /*dst->*/dst_ip += 9;
                    last_offset = 9;
                    break;

                case 5: // SUB
                    /*dst->*/src_ip += sizeof(char);
                    /*dst->*/dst_ip += 9;
                    last_offset = 9;
                    break;

                case 6: // MUL
                    /*dst->*/src_ip += sizeof(char);
                    /*dst->*/dst_ip += 17;
                    last_offset = 17;
                    break;

                case 7: // DIV
                    /*dst->*/src_ip += sizeof(char);
                    /*dst->*/dst_ip += 22;
                    last_offset = 22;
                    break;
                case 8: // POP
                    if ((src_arr[/*dst->*/src_ip] & IS_REG) && (src_arr[/*dst->*/src_ip] & IS_RAM))
                        {
                            /*dst->*/src_ip++;

                            switch (src_arr[/*dst->*/src_ip++])
                            {
                                case (2 | IS_REG):
                                    /*dst->*/src_ip += 2 * sizeof(int) + sizeof(char);
                                    /*dst->*/dst_ip += 18;
                                    last_offset = 18;
                                    break;

                                case (2 | IS_RAM | IS_REG):
                                    /*dst->*/src_ip += 2 * sizeof(int) + sizeof(char);
                                    /*dst->*/dst_ip += 18;
                                    last_offset = 18;
                                    break;

                                case (1 | IS_REG):
                                    /*dst->*/src_ip += sizeof(int);
                                    /*dst->*/dst_ip += 9;
                                    last_offset = 9;
                                    break;

                                case (2 | IS_RAM):
                                    /*dst->*/src_ip += 2 * sizeof(int) + sizeof(char);
                                    /*dst->*/dst_ip += 18;
                                    last_offset = 18;
                                    break;

                                case (2):
                                    /*dst->*/src_ip += 2 * sizeof(int) + sizeof(char);
                                    /*dst->*/dst_ip += 12;
                                    last_offset = 18;
                                    break;
                            }
                        }
                    else if (src_arr[/*dst->*/src_ip] & IS_REG) // for PUSH reg
                    {
                        /*dst->*/src_ip += sizeof(unsigned) + sizeof(char);
                        /*dst->*/dst_ip++;
                        last_offset = 1;
                    }
                    else if (src_arr[/*dst->*/src_ip] & IS_RAM)
                    {
                        /*dst->*/src_ip += sizeof(unsigned) + sizeof(char);
                        /*dst->*/dst_ip += 12;
                        last_offset = 12;
                    }
                    else    // for PUSH num
                    {
                        /*dst->*/src_ip += sizeof(char);
                        /*dst->*/dst_ip += 2;
                        last_offset = 2;
                    }
                    break;

                case 9:// JMP
                    /*dst->*/src_ip += sizeof(char) + sizeof(unsigned);
                    /*dst->*/dst_ip += sizeof(char) + sizeof(int);
                    last_offset = sizeof(char) + sizeof(int);
                    break; 

                case 10:    // MRK
                    src_ip++;
                    break;

                case 11: case 12: case 13: case 14: case 15: case 16:
                    PRINT_LINE;
                    src_ip += sizeof(char) + sizeof(unsigned);
                    dst_ip += sizeof(char) * 2 + sizeof(int) + sizeof(char) * 7;
                    last_offset = 13;
                    break;

                case 17:    // CALL
                    src_ip += sizeof(char) + sizeof(int);
                    dst_ip += sizeof(char) + sizeof(int);
                    last_offset = sizeof(char) + sizeof(int);
                    break;

                case 18:
                    src_ip++;
                    dst_ip++;
                    last_offset = 1;
                    break;

                default:
                    fprintf(stderr, "Unknown oper %d\n", src_arr[src_ip]);
                    PRINT_LINE;
                    exit(1);
            }
        }
        
        unsigned char cmd_code = dst->buffer[dst->labels.data[lab_num].dst_ip - 1];

        fprintf(stderr, "(cmd_code == 0xE9 || cmd_code == 0xE8) = %d\ncmd_code = %x\n", (cmd_code == 0xE9 || cmd_code == 0xE8), cmd_code);
        fprintf(stderr, "dst_ip = %lu, lab ip = %d\n", dst_ip, dst->labels.data[lab_num].dst_ip);
        fprintf(stderr, "last offset = %d\n", last_offset);

        int lab_dst_ip = dst->labels.data[lab_num].dst_ip;

        int offset = 0;

        if (dst_ip > lab_dst_ip || cmd_code == 0xE8)
        {
            PRINT_LINE;
            offset = (int) ((int) dst_ip - lab_dst_ip - sizeof(int));
        }
        else
        {
            offset = (int) ((int) dst_ip - lab_dst_ip - sizeof(char) - sizeof(int) - (sizeof(char) * 8) * !(cmd_code == 0xE9 || cmd_code == 0xE8) + (dst_ip > lab_dst_ip));
        }

        unsigned offset_uns = (unsigned) offset;

        fprintf(stderr, "offset = %d, %x\n", offset, offset_uns);

        //*((unsigned *) (dst->buffer + dst->labels.data[lab_num].dst_ip)) = offset_uns;

        for (int j = 0; j < sizeof(int); ++j)
        {
            dst->buffer[j + dst->labels.data[lab_num].dst_ip] = offset_uns % 0x100;
            offset_uns /= 0x100;
        }
    }

    return 0;
}


static int makeJmp(unsigned char *src, Bin_code *dst, int jmp_code, int curr_symb)
{
    is_debug(if (!dst)  ERR(INVALID_PTR));

    switch (jmp_code)
    {
        case 9:
            FILL1BYTE(0xE9);
            fprintf(dst->asm_version, "jmp <addr>\n"); fflush(dst->asm_version);
            break;
        
        case 11:
            CMP_R13_R15;
            FILL1BYTE(0x0F);
            FILL1BYTE(0x87);
            fprintf(dst->asm_version, "ja  <addr>\n"); fflush(dst->asm_version);
            break;

        case 12:
            CMP_R13_R15;
            FILL1BYTE(0x0F);
            FILL1BYTE(0x83);
            fprintf(dst->asm_version, "jae <addr>\n"); fflush(dst->asm_version);
            break;

        case 13:
            CMP_R13_R15;
            FILL1BYTE(0x0F);
            FILL1BYTE(0x82);
            fprintf(dst->asm_version, "jb  <addr>\n"); fflush(dst->asm_version);
            break;

        case 14:
            CMP_R13_R15;
            FILL1BYTE(0x0F);
            FILL1BYTE(0x86);
            fprintf(dst->asm_version, "jbe <addr>\n"); fflush(dst->asm_version);
            break;

        case 15:
            CMP_R13_R15;
            FILL1BYTE(0x0F);
            FILL1BYTE(0x84);
            fprintf(dst->asm_version, "je  <addr>\n"); fflush(dst->asm_version);
            break;
        
        case 16:
            CMP_R13_R15;
            FILL1BYTE(0x0F);
            FILL1BYTE(0x85);
            fprintf(dst->asm_version, "jne <addr>\n"); fflush(dst->asm_version);
            break;
    }

    labelPushBack(dst, *((int *)(src + 1)));
    dst->length += sizeof(int);
    
    //writeNumber(dst, (u_int64_t) &makeJump);

    return 0;
}


int LabComp(const void *first, const void *second)
{
    is_debug(if (!first || !second) ERR(INVALID_PTR));

    Label *First  = (Label *) first;
    Label *Second = (Label *) second;

    return First->src_ip > Second->src_ip;
}


static int makeCall(unsigned char *src, Bin_code *dst)
{
    is_debug(if (!src || !dst)  ERR(INVALID_PTR));

    FILL1BYTE(0xE8);

    labelPushBack(dst, *((int *) (src + 1)));
    dst->length += sizeof(int);

    fprintf(dst->asm_version, "call <addr>\n");

    return 0;
}