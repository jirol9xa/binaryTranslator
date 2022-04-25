#include "stdio.h"
#include "stdlib.h"
#include "translator.h"
#include "reader.h"


static long fileLength(FILE *fp);


int reader(FILE *src_file, Sourse_code *src)
{
    is_debug(if (!src_file)    ERR(INVALID_FILE);
             if (!src)         ERR(INVALID_PTR)  ;)

    long length = fileLength(src_file);

    src->buffer = (char *) calloc(length, sizeof(char));
    is_debug(if (!src)  ERR(MEM_OVERFLOW))

    long rl_length = fread(src, sizeof(char), length, src_file);

    if (length != rl_length)
    {
        void *temp_ptr = realloc(src->buffer, (rl_length + 1) * sizeof(char));
        is_debug(if (!temp_ptr)     ERR(MEM_OVERFLOW))

        src->buffer = (char *) temp_ptr;

        fprintf(stderr, "FSEEK returned the wrong length\n");
    }

    src->buffer[rl_length] = '\0';
    src->length = rl_length + 1;

    return 0;
}


static long fileLength(FILE *fp)
{
    is_debug(if (!fp)    ERR(INVALID_PTR))

    long length = 0;

    #define FSEEK(param)                                \
    {                                                   \
        if (fseek(fp, 0L, SEEK_##param))                \
        {                                               \
            printf("In %s fseek failed\n", __func__);   \
            ERR(FUNC_FAILED);                           \
        }                                               \
    }

    FSEEK(END);
    length = ftell(fp);
    FSEEK(SET);

    #undef FSEEK

    return length;
}