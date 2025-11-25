#ifndef KCLILIB_H_
#define KCLILIB_H_

#include <stdbool.h>
#include <stdlib.h>

struct kcli_option
{
    char short_name;
    char const *long_name;
    char const *pos_name;

    bool *ptr_flag;
    char const **ptr_str;
    long *ptr_long;
};

bool kcli_parse(
    struct kcli_option const *opts,
    size_t count,
    int argc,
    char const *const *argv
);

#define KCLI_COUNTOF(arr) (sizeof(arr) / sizeof((arr)[0]))

#define KCLI_PARSE(argc, argv, ...)                                            \
    do                                                                         \
    {                                                                          \
        struct kcli_option opts_[] = {__VA_ARGS__};                            \
                                                                               \
        bool const ok_ =                                                       \
            kcli_parse(opts_, KCLI_COUNTOF(opts_), (argc), (argv));            \
        if (!ok_)                                                              \
        {                                                                      \
            exit(1);                                                           \
        }                                                                      \
    } while (0)

#endif
