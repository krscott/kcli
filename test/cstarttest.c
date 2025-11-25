#include "cstartlib.h"
#include <assert.h>
#include <stdio.h>

static void check_ok(void)
{
    long out;
    enum cstart_strsum_err err = cstart_strsum("11", "22", &out);
    assert(err == CSTART_STRSUM_OK);
    assert(out == 33);
}

static void check_fail_a(void)
{
    long out;
    enum cstart_strsum_err err = cstart_strsum("11a", "22", &out);
    assert(err == CSTART_STRSUM_NOT_AN_INT_A);
}

static void check_fail_b(void)
{
    long out;
    enum cstart_strsum_err err = cstart_strsum("11", "foo", &out);
    assert(err == CSTART_STRSUM_NOT_AN_INT_B);
}

#define RUN(test)                                                              \
    do                                                                         \
    {                                                                          \
        printf("Test: " #test "\n");                                           \
        test();                                                                \
    } while (0)

int main(void)
{
    RUN(check_ok);
    RUN(check_fail_a);
    RUN(check_fail_b);

    return 0;
}
