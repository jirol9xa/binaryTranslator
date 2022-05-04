#include "stdio.h"
#include <sys/mman.h>
#include "errno.h"
#include "stdlib.h"
#include "translator.h"
#include "reader.h"


int main(const int argc, const char *argv[])
{
    // if (argc < 2)
    // {
    //     printf("Not enough args of cmd line\n");
    //     return 0;
    // }

    system("cd .. && cd cpu && cd ASM && ./asm text");

    FILE *src_file = fopen("../cpu/ASM/Binary"/*argv[1]*/, "rb");
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

    if (mprotect(binary.buffer, binary.length, PROT_EXEC | PROT_WRITE))
    {
        perror("Can't make mprotect\n");
        exit(errno);
    }

    PRINT_LINE;

    void (*Pup) (void);
    Pup = (void (*) (void)) binary.buffer;
    fprintf(stderr, "Pup addr = %lx\n", (u_int64_t) Pup);
    Pup();


    fprintf(stderr, "buff addr = %p\n", &binary.buffer);

    SourceDtor(&src);
    BinDtor(&binary);

    return 0;
}