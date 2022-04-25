#include "stdio.h"
#include "stdlib.h"


int main()
{
    unsigned *arr = (unsigned *) calloc(1, sizeof(unsigned));
    unsigned  arg = 0x1234;

    *((char *) arr) = arg / 0x10000;
    arg %= 0x10000;
    *((char *) arr + 1) = arg / 0x100;
    arg %= 0x100;
    *((char *) arr + 2) = arg / 0x1;
    arg %= 0x1;
    *((char *) arr + 3) = 0x68;

    *arr = 0xABCDABCD;
    //*((char *) arr + 1) = 0x98;
    //*((char *) arr + 2) = 0x12;
    //*((char *) arr + 3) = 0x56;
    fprintf(stderr, "%x\n", *((char *) arr + 0));
    printf("%x\n", *arr);

    return 0;
}