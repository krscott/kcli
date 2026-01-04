#include "kcli.inc"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define STR_EQ(a, b) (strcmp(a, b) == 0)
#define DBL_EQ(a, b) ((a) + 0.01 > (b) && (b) + 0.01 > (a))

static void t_opt_pos(void)
{
    char const *argv[] = {"test", "foo", "bar"};
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
    char const *argv[] = {"test", "foo", "bar"};
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
    char const *argv[] = {"test", "foo"};
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

static void t_optional_pos(void)
{
    char const *argv[] = {"test", "foo"};
    int const argc = KCLI_COUNTOF(argv);

    char const *alpha;
    char const *bravo = "default";

    KCLI_PARSE(
        argc,
        argv,
        {.pos_name = "alpha", .ptr_str = &alpha},
        {.pos_name = "bravo", .ptr_str = &bravo, .optional = true},
    );

    assert(STR_EQ(alpha, "foo"));
    assert(STR_EQ(bravo, "default"));
}

static void t_opt_long_flags(void)
{
    char const *argv[] = {"test", "--bravo"};
    int const argc = KCLI_COUNTOF(argv);

    bool alpha = false;
    bool bravo = false;

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
    char const *argv[] = {"test", "-b"};
    int const argc = KCLI_COUNTOF(argv);

    bool alpha = false;
    bool bravo = false;

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
    char const *argv[] = {"test", "-a", "--bravo"};
    int const argc = KCLI_COUNTOF(argv);

    bool alpha = false;
    bool bravo = false;
    bool charlie = false;

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
    char const *argv[] = {"test", "-a=foo", "-bbar", "-c", "qux"};
    int const argc = KCLI_COUNTOF(argv);

    char const *alpha_str = NULL;
    char const *bravo_str = NULL;
    char const *charlie_str = NULL;

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
    char const *argv[] = {"test", "--alpha=foo", "--bravo", "bar"};
    int const argc = KCLI_COUNTOF(argv);

    char const *alpha_str = NULL;
    char const *bravo_str = NULL;

    KCLI_PARSE(
        argc,
        argv,
        {.long_name = "alpha", .ptr_str = &alpha_str},
        {.long_name = "bravo", .ptr_str = &bravo_str},
    );

    assert(STR_EQ(alpha_str, "foo"));
    assert(STR_EQ(bravo_str, "bar"));
}

static void t_multi_short(void)
{
    char const *argv[] = {"test", "-abcbad"};
    int const argc = KCLI_COUNTOF(argv);

    bool alpha_flag = false;
    bool bravo_flag = false;
    char const *charlie_str = NULL;
    bool delta_flag = false;

    KCLI_PARSE(
        argc,
        argv,
        {.short_name = 'a', .ptr_flag = &alpha_flag},
        {.short_name = 'b', .ptr_flag = &bravo_flag},
        {.short_name = 'c', .ptr_str = &charlie_str},
        {.short_name = 'd', .ptr_flag = &delta_flag},
    );

    assert(alpha_flag);
    assert(bravo_flag);
    assert(STR_EQ(charlie_str, "bad"));
    assert(!delta_flag);
}

static void t_int_arg(void)
{
    char const *argv[] = {"test", "-a51", "123", "--bravo=-100"};
    int const argc = KCLI_COUNTOF(argv);

    long alpha = 0;
    long bravo = 0;
    long charlie = 0;
    long delta = 0;

    KCLI_PARSE(
        argc,
        argv,
        {.short_name = 'a', .ptr_i64 = &alpha},
        {.long_name = "bravo", .ptr_i64 = &bravo},
        {.long_name = "charlie", .ptr_i64 = &charlie},
        {.pos_name = "delta", .ptr_i64 = &delta},
    );

    assert(alpha == 51);
    assert(bravo == -100);
    assert(charlie == 0);
    assert(delta == 123);
}

static void t_float_arg(void)
{
    char const *argv[] = {"test", "-a-0.5", "100.5", "--bravo=99.5"};
    int const argc = KCLI_COUNTOF(argv);

    double alpha = 0.0;
    double bravo = 0.0;
    double charlie = 0.0;
    double delta = 0.0;

    KCLI_PARSE(
        argc,
        argv,
        {.short_name = 'a', .ptr_f64 = &alpha},
        {.long_name = "bravo", .ptr_f64 = &bravo},
        {.long_name = "charlie", .ptr_f64 = &charlie},
        {.pos_name = "delta", .ptr_f64 = &delta},
    );

    assert(DBL_EQ(alpha, -0.5));
    assert(DBL_EQ(bravo, 99.5));
    assert(charlie == 0.0);
    assert(DBL_EQ(delta, 100.5));
}

static void t_double_dash(void)
{
    char const *argv[] = {"test", "-afoo", "--", "-a", "-1"};
    int const argc = KCLI_COUNTOF(argv);

    char const *alpha = NULL;
    char const *bravo = NULL;
    long charlie = 0;

    KCLI_PARSE(
        argc,
        argv,
        {.short_name = 'a', .ptr_str = &alpha},
        {.pos_name = "bravo", .ptr_str = &bravo},
        {.pos_name = "charlie", .ptr_i64 = &charlie},
    );

    assert(STR_EQ(alpha, "foo"));
    assert(STR_EQ(bravo, "-a"));
    assert(charlie == -1);
}

static void t_help(void)
{
    bool help = false;
    char const *alpha = NULL;
    char const *bravo = NULL;
    char const *charlie = NULL;
    char const *delta = "delta-default";

    struct kcli_option opts[] = {
        {
            .short_name = 'h',
            .long_name = "help",
            .help = "Show this help and exit",
            .ptr_flag = &help,
        },
        {
            .pos_name = "alpha",
            .ptr_str = &alpha,
            .help = "Argument alpha",
        },
        {
            .short_name = 'b',
            .ptr_str = &bravo,
            .help = "Argument beta",
        },
        {
            .long_name = "charlie-extra-long",
            .ptr_str = &charlie,
            .help = "Argument charlie",
        },
        {
            .pos_name = "delta",
            .ptr_str = &delta,
            .help = "Argument delta",
            .optional = true,
        },
    };

    kcli_print_help("t_help", opts, KCLI_COUNTOF(opts));
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
    RUN(t_optional_pos);
    RUN(t_opt_long_flags);
    RUN(t_opt_short_flags);
    RUN(t_opt_long_or_short);
    RUN(t_short_arg);
    RUN(t_long_arg);
    RUN(t_multi_short);
    RUN(t_int_arg);
    RUN(t_float_arg);
    RUN(t_double_dash);
    RUN(t_help);

    return 0;
}
