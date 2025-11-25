#include "kclilib.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define STR_EQ(a, b) (strcmp(a, b) == 0)

static void t_opt_pos(void)
{
    char const *argv[] = {"foo", "bar"};
    int const argc = KCLI_COUNTOF(argv);

    char const *alpha;
    char const *bravo;

    KCLI_PARSE(
        argc,
        argv,
        {.pos_name = "alpha", .ptr_str = &alpha},
        {.pos_name = "bravo", .ptr_str = &bravo},
    );

    assert(STR_EQ(alpha, "foo"));
    assert(STR_EQ(bravo, "bar"));
}

static void t_too_many_pos(void)
{

    char const *argv[] = {"foo", "bar"};
    int const argc = KCLI_COUNTOF(argv);

    char const *alpha;

    struct kcli_option opts[] = {
        {.pos_name = "alpha", .ptr_str = &alpha},
    };

    bool const ok = kcli_parse(opts, KCLI_COUNTOF(opts), (argc), (argv));
    assert(!ok);
}

// static void t_opt_flags(void)
// {
//     char *argv[] = {"--alpha", "-c"};
//     int const argc = KCLI_COUNTOF(argv);
//
//     KCLI_PARSE(
//         argc,
//         argv,
//         {.short_name = 'a', .long_name = "--alpha"},
//         {.short_name = 'b', .long_name = "--bravo"},
//     );
// }

#define RUN(test)                                                              \
    do                                                                         \
    {                                                                          \
        printf("Test: " #test "\n");                                           \
        test();                                                                \
    } while (0)

int main(void)
{
    RUN(t_opt_pos);
    RUN(t_too_many_pos);
    // RUN(t_opt_flags);

    return 0;
}
