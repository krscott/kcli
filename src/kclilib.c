#include "kclilib.h"

#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#define error(msg) printf("CLI Error: " msg "\n")
#define errorf(fmt, ...) printf("CLI Error: " fmt "\n", __VA_ARGS__)

static bool str2int(char const *const str, long *const out)
{
    assert(str);

    char *end = NULL;

    errno = 0;
    *out = strtol(str, &end, 10);

    assert(end);
    return (errno == 0) && (*end == '\0');
}

static void
set_opt_ptr(struct kcli_option const *const opt, char const *const str)
{
    if (opt->ptr_str)
    {
        *(opt->ptr_str) = str;
    }
    else
    {
        assert(false);
    }
}

static bool set_nth_positional(
    struct kcli_option const *const opts,
    size_t const count,
    size_t n,
    char const *const arg
)
{
    bool ok = false;

    for (size_t i = 0; i < count; ++i)
    {
        struct kcli_option const *const opt = &opts[i];

        if (opt->pos_name)
        {
            if (n == 0)
            {
                set_opt_ptr(opt, arg);
                ok = true;
                break;
            }
            else
            {
                --n;
            }
        }
    }

    return ok;
}

bool kcli_parse(
    struct kcli_option const *const opts,
    size_t const count,
    int const argc,
    char const *const *const argv
)
{
    assert(opts);
    assert(count >= 0);
    assert(argv);
    assert(argc >= 0);

    bool ok = true;

    size_t positional = 0;

    for (int i = 0; i < argc; ++i)
    {
        char const *const arg = argv[i];

        if (!set_nth_positional(opts, count, positional++, arg))
        {
            error("Too many positional arguments");
            ok = false;
            break;
        }
    }

    return ok;
}
