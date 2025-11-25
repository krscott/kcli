#include "cstartlib.h"
#include <stdio.h>

int main(int const argc, char const *const *argv)
{
    int err = 0;
    long sum;

    if (argc != 3)
    {
        err = 1;
        printf("Usage: %s INT INT\n", argv[0]);
        goto error;
    }

    switch (cstart_strsum(argv[1], argv[2], &sum))
    {
        case CSTART_STRSUM_OK:
            printf("%s + %s = %ld\n", argv[1], argv[2], sum);
            break;
        case CSTART_STRSUM_NOT_AN_INT_A:
            printf("Not an int: %s\n", argv[1]);
            err = 2;
            break;
        case CSTART_STRSUM_NOT_AN_INT_B:
            printf("Not an int: %s\n", argv[2]);
            err = 2;
            break;
    }
    {
        err = 2;
        goto error;
    }

error:
    return err;
}
