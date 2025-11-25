#ifndef KCLILIB_H_
#define KCLILIB_H_

#include <stdbool.h>

enum kcli_strsum_err
{
    KCLI_STRSUM_OK,
    KCLI_STRSUM_NOT_AN_INT_A,
    KCLI_STRSUM_NOT_AN_INT_B,
};

enum kcli_strsum_err kcli_strsum(char const *a, char const *b, long *out);

#endif
