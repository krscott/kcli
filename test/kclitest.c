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

static void t_not_enough_pos(void)
{
    char const *argv[] = {"foo"};
    int const argc = KCLI_COUNTOF(argv);

    char const *alpha;
    char const *bravo;

    struct kcli_option opts[] = {
        {.pos_name = "alpha", .ptr_str = &alpha},
        {.pos_name = "bravo", .ptr_str = &bravo},
    };

    bool const ok = kcli_parse(opts, KCLI_COUNTOF(opts), (argc), (argv));
    assert(!ok);
}

static void t_opt_long_flags(void)
{
    char const *argv[] = {"--bravo"};
    int const argc = KCLI_COUNTOF(argv);

    bool alpha;
    bool bravo;

    KCLI_PARSE(
        argc,
        argv,
        {.long_name = "alpha", .ptr_flag = &alpha},
        {.long_name = "bravo", .ptr_flag = &bravo},
    );

    assert(!alpha);
    assert(bravo);
}

static void t_opt_short_flags(void)
{
    char const *argv[] = {"-b"};
    int const argc = KCLI_COUNTOF(argv);

    bool alpha;
    bool bravo;

    KCLI_PARSE(
        argc,
        argv,
        {.short_name = 'a', .ptr_flag = &alpha},
        {.short_name = 'b', .ptr_flag = &bravo},
    );

    assert(!alpha);
    assert(bravo);
}

static void t_opt_long_or_short(void)
{
    char const *argv[] = {"-a", "--bravo"};
    int const argc = KCLI_COUNTOF(argv);

    bool alpha;
    bool bravo;
    bool charlie;

    KCLI_PARSE(
        argc,
        argv,
        {.short_name = 'a', .long_name = "alpha", .ptr_flag = &alpha},
        {.short_name = 'b', .long_name = "bravo", .ptr_flag = &bravo},
        {.short_name = 'c', .long_name = "charlie", .ptr_flag = &charlie},
    );

    assert(alpha);
    assert(bravo);
    assert(!charlie);
}

static void t_short_arg(void)
{
    char const *argv[] = {"-a=foo", "-bbar", "-c", "qux"};
    int const argc = KCLI_COUNTOF(argv);

    char const *alpha_str;
    char const *bravo_str;
    char const *charlie_str;

    KCLI_PARSE(
        argc,
        argv,
        {.short_name = 'a', .ptr_str = &alpha_str},
        {.short_name = 'b', .ptr_str = &bravo_str},
        {.short_name = 'c', .ptr_str = &charlie_str},
    );

    assert(STR_EQ(alpha_str, "foo"));
    assert(STR_EQ(bravo_str, "bar"));
    assert(STR_EQ(charlie_str, "qux"));
}

static void t_long_arg(void)
{
    char const *argv[] = {"--alpha=foo", "--bravo", "bar"};
    int const argc = KCLI_COUNTOF(argv);

    char const *alpha_str;
    char const *bravo_str;

    KCLI_PARSE(
        argc,
        argv,
        {.long_name = "alpha", .ptr_str = &alpha_str},
        {.long_name = "bravo", .ptr_str = &bravo_str},
    );

    assert(STR_EQ(alpha_str, "foo"));
    assert(STR_EQ(bravo_str, "bar"));
}

#define RUN(test)                                                              \
    do                                                                         \
    {                                                                          \
        printf("Test: " #test "\n");                                           \
        fflush(stdout);                                                        \
        test();                                                                \
    } while (0)

int main(void)
{
    RUN(t_opt_pos);
    RUN(t_too_many_pos);
    RUN(t_not_enough_pos);
    RUN(t_opt_long_flags);
    RUN(t_opt_short_flags);
    RUN(t_opt_long_or_short);
    RUN(t_short_arg);
    RUN(t_long_arg);

    return 0;
}
