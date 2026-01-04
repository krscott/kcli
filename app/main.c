#include "kcli.h"
#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

struct opts
{
    bool verbose;
    int64_t times;
    char const *names[10];
    size_t names_count;
};

static struct opts opts_parse(int const argc, char const *const *const argv)
{
    struct opts opts = {.times = 1};

    KCLI_PARSE(
        argc,
        argv,
        {
            .name = "name",
            .ptr_str = opts.names,
            .ptr_nargs = &opts.names_count,
            .nargs_max = KCLI_COUNTOF(opts.names),
            .help = "Name (can specify up to 10)",
        },
        {
            .name = "N",
            .short_name = 'n',
            .long_name = "times",
            .ptr_i64 = &opts.times,
            .help = "How many times to print (default: 1)",
        },
        {
            .short_name = 'v',
            .long_name = "verbose",
            .ptr_flag = &opts.verbose,
            .help = "Enable extra logging",
        },
    );

    if (opts.times < 0 || opts.times > 20)
    {
        printf("--times arg must be between 0 and 20\n");
        exit(1);
    }

    return opts;
}

int main(int const argc, char const *const *const argv)
{
    struct opts opts = opts_parse(argc, argv);

    if (opts.verbose)
    {
        // NOTE: 'name' is not optional, so names_count will always be >=1 here
        //  (enforced by KCLI_PARSE)
        printf("Printing hello to %s...\n", opts.names[0]);
    }

    for (size_t i = 0; i < (size_t)opts.times; ++i)
    {
        printf("Hello");
        for (size_t j = 0; j < opts.names_count; ++j)
        {
            printf(" %s", opts.names[j]);
        }
        printf("!\n");
    }

    return 0;
}
