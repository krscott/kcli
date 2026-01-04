#ifndef KCLILIB_H_
#define KCLILIB_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

struct kcli_option
{
    char short_name;
    char const *long_name;
    char const *name;
    char const *help;

    bool optional;
    size_t nargs_max;
    size_t *ptr_nargs;

    bool *ptr_flag;
    char const **ptr_str;
    int64_t *ptr_i64;
    double *ptr_f64;

    bool _is_used;
    bool _is_partially_used;
};

bool kcli_parse(
    struct kcli_option *opts,
    size_t count,
    int argc,
    char const *const *argv,
    char *err_buf,
    size_t err_buf_size
);

void kcli_print_usage(
    FILE *stream,
    char const *prog_name,
    struct kcli_option const *opts,
    size_t count
);

void kcli_print_help(
    char const *prog_name, struct kcli_option const *opts, size_t count
);

#define KCLI_COUNTOF(arr) (sizeof(arr) / sizeof((arr)[0]))

#define KCLI_PARSE(argc, argv, ...)                                            \
    do                                                                         \
    {                                                                          \
        bool help_ = false;                                                    \
                                                                               \
        struct kcli_option opts_[] = {                                         \
            {                                                                  \
                .short_name = 'h',                                             \
                .long_name = "help",                                           \
                .help = "Show this help and exit",                             \
                .ptr_flag = &help_,                                            \
            },                                                                 \
            __VA_ARGS__                                                        \
        };                                                                     \
                                                                               \
        char err_buf_[256];                                                    \
                                                                               \
        bool const ok_ = kcli_parse(                                           \
            opts_,                                                             \
            KCLI_COUNTOF(opts_),                                               \
            (argc),                                                            \
            (argv),                                                            \
            err_buf_,                                                          \
            sizeof(err_buf_)                                                   \
        );                                                                     \
                                                                               \
        if (help_)                                                             \
        {                                                                      \
            kcli_print_help(argv[0], opts_, KCLI_COUNTOF(opts_));              \
            exit(0);                                                           \
        }                                                                      \
                                                                               \
        if (!ok_)                                                              \
        {                                                                      \
            fprintf(stderr, "%.*s\n", (int)sizeof(err_buf_), err_buf_);        \
            kcli_print_usage(stderr, argv[0], opts_, KCLI_COUNTOF(opts_));     \
            exit(1);                                                           \
        }                                                                      \
    } while (0)

#endif
