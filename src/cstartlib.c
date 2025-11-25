#include "cstartlib.h"

#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

static bool str2int(char const *const str, long *const out)
{
    assert(str);

    char *end = NULL;

    errno = 0;
    *out = strtol(str, &end, 10);

    assert(end);
    return (errno == 0) && (*end == '\0');
}

enum cstart_strsum_err
cstart_strsum(char const *a, char const *b, long *const out)
{
    bool ok = true;
    enum cstart_strsum_err err = CSTART_STRSUM_OK;

    long ai;
    long bi;

    *out = 0;

    ok = str2int(a, &ai);
    if (!ok)
    {
        err = CSTART_STRSUM_NOT_AN_INT_A;
        goto error;
    }

    ok = str2int(b, &bi);
    if (!ok)
    {
        err = CSTART_STRSUM_NOT_AN_INT_B;
        goto error;
    }

    *out = ai + bi;

error:
    return err;
}
