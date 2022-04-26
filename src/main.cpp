#include "stdio.h"
#include <sys/mman.h>
#include "translator.h"
#include "reader.h"

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

    fprintf(stderr, "buff length = %ld\n", binary.length);
    fprintf(stderr, "mprot = %d", mprotect(binary.buffer, binary.length, PROT_EXEC));

    void (*Pup) (void);
    Pup = (void (*) (void)) binary.buffer;
    Pup();

    SourceDtor(&src);
    BinDtor(&binary);
    return 0;
}