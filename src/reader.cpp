#include "stdio.h"
#include "stdlib.h"
#include "translator.h"
#include "reader.h"


const int Header_size = 20;


static long fileLength(FILE *fp);


int reader(FILE *src_file, Sourse_code *src)
{
    is_debug(if (!src_file)    ERR(INVALID_FILE);
             if (!src)         ERR(INVALID_PTR)  ;)

    long length = fileLength(src_file);

    fprintf(stderr, "length = %ld\n", length);

    src->buffer = (unsigned char *) calloc(length + 1, sizeof(unsigned char));
    is_debug(if (!src)  ERR(MEM_OVERFLOW))

    long rl_length = fread(src->buffer, sizeof(unsigned char), length, src_file);

    if (length != rl_length)
    {
        fprintf(stderr, "rl_length = %ld, length = %ld\n", rl_length, length);

        void *temp_ptr = realloc(src->buffer, (rl_length + 1) * sizeof(char));
        is_debug(if (!temp_ptr)     ERR(MEM_OVERFLOW))

        src->buffer = (unsigned char *) temp_ptr;

        fprintf(stderr, "FSEEK returned the wrong length\n");
    }

    src->buffer[rl_length] = '\0';
    src->length = rl_length;

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

    if (fseek(fp, 0L, SEEK_END))                
    {                                               
        printf("In %s fseek failed\n", __func__);   
        ERR(FUNC_FAILED);                           
    }                                               
    
    length = ftell(fp) - Header_size;
    
    if (fseek(fp, (long) Header_size, SEEK_SET))                
    {                                               
        printf("In %s fseek failed\n", __func__);   
        ERR(FUNC_FAILED);                           
    } 

    #undef FSEEK

    return length;
}