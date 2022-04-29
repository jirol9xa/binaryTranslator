#ifndef TRANSLATOR_H
    #define TRANSLATOR_H
    
    struct Sourse_code
    {
        unsigned char *buffer;
        long           length;
    };

    struct Label
    {
        long  src_ip;
        char *dst_place;
    };

    struct Labels_arr
    {
        Label *data;
        int    capacity;
        int    size;
    };

    struct Bin_code
    {
        unsigned char *buffer;
        long           length;
        long           capacity;
        FILE          *asm_version;
        long           src_ip;
        long           dst_ip;
        Labels_arr     labels;
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

    int SourceDtor(Sourse_code *src);
    int BinCtor   (Bin_code *dst, long buff_length);
    int BinDtor   (Bin_code *dst);

    int translation(Sourse_code *src, Bin_code *dst);

#endif