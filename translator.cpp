#include "stdlib.h"
#include "stdio.h"
#include "translator.h"
#include "reader.h"

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
        switch (src_arr[i])
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



    return 0;
}