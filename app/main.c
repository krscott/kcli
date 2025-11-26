#include "kclilib.h"
#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

struct opts
{
    bool verbose;
    long times;
    char const *first;
    char const *last;
};

static struct opts opts_parse(int const argc, char const *const *const argv)
{
    struct opts opts = {.times = 1};

    KCLI_PARSE(
        argc,
        argv,
        {
            .pos_name = "first",
            .ptr_str = &opts.first,
            .help = "First name",
        },
        {
            .pos_name = "last",
            .ptr_str = &opts.last,
            .optional = true,
            .help = "Last name",
        },
        {
            .short_name = 'n',
            .long_name = "times",
            .ptr_long = &opts.times,
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
        printf("Printing hello...\n");
    }

    for (size_t i = 0; i < (size_t)opts.times; ++i)
    {
        assert(opts.first); // enforced by KCLI_PARSE
        printf("Hello, %s", opts.first);

        if (opts.last)
        {
            printf(" %s", opts.last);
        }

        printf("!\n");
    }

    return 0;
}
