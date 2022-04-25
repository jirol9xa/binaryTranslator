#ifndef TRANSLATOR_H
    #define TRANSLATOR_H
    
    struct Sourse_code
    {
        char *buffer;
        long  length;
    };

    struct Bin_code
    {
        char *buffer;
        long  length;
        FILE *asm_version;
    };

    enum ERRORS
    {
        INVALID_FILE  = -1,
        INVALID_PTR   = -2,
        FUNC_FAILED   = -3,
        MEM_OVERFLOW  = -4,
    };


    #define DEBUG 1         // if debug mode 1, else 0

    #define is_debug(code)              \
    {                                   \
        if (DEBUG)                      \
        {                               \
            code;                       \
        }                               \
    }

    #define ERR(param)                              \
    {                                               \
        fprintf(stderr, "!!!ERROR %s!!! ", #param); \
        PRINT_LINE;                                 \
        return param;                               \
    }

    #define PRINT_LINE  fprintf(stderr, "[%s:%d]\n", __func__, __LINE__); 

#endif