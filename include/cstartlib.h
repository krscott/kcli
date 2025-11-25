#ifndef CSTARTLIB_H_
#define CSTARTLIB_H_

#include <stdbool.h>

enum cstart_strsum_err
{
    CSTART_STRSUM_OK,
    CSTART_STRSUM_NOT_AN_INT_A,
    CSTART_STRSUM_NOT_AN_INT_B,
};

enum cstart_strsum_err cstart_strsum(char const *a, char const *b, long *out);

#endif
