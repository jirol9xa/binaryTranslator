#include "stdio.h"


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

    


    return 0;
}