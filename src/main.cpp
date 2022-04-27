#include "stdio.h"
#include <sys/mman.h>
#include "errno.h"
#include "stdlib.h"
#include "translator.h"
#include "reader.h"
#include "limits.h"

#ifndef PAGESIZE
    #define PAGESIZE 4096
#endif


int main(const int argc, const char *argv[])
{
    if (argc < 2)
    {
        printf("Not enough args of cmd line\n");
        return 0;
    }

    FILE *src_file = fopen(argv[1], "rb");
    if (!src_file)
    {
        printf("Can't open %s file in binary read mode\n", argv[1]);
        return 0;
    }

    Sourse_code src = {};
    reader(src_file, &src);

    Bin_code binary = {};
    BinCtor(&binary, src.length);

    translation(&src, &binary);

    for (int i = 0; i < binary.length; ++i)
    {
        printf("%x ", binary.buffer[i]);
    }
    printf("\n");

    fprintf(stderr, "buff length = %ld\n", binary.length);

    posix_memalign((void **) &(binary.buffer), PAGESIZE, binary.length);

    if (mprotect(binary.buffer, binary.length, PROT_WRITE))
    {
        perror("Can't make mprotect\n");
        exit(errno);
    }
    if (mprotect(binary.buffer, binary.length, PROT_EXEC))
    {
        perror("Can't make mprotect\n");
        exit(errno);
    }

    PRINT_LINE;

    void (*Pup) (void);
    Pup = (void (*) (void)) binary.buffer;
    PRINT_LINE;
    Pup();

    PRINT_LINE;

    SourceDtor(&src);
    BinDtor(&binary);
    return 0;
}