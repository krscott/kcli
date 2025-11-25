#include "kclilib.h"

#include <assert.h>
// #include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define error(msg) printf("CLI Error: " msg "\n")
#define errorf(fmt, ...) printf("CLI Error: " fmt "\n", __VA_ARGS__)

#if 0
static bool str2int(char const *const str, long *const out)
{
    assert(str);

    char *end = NULL;

    errno = 0;
    *out = strtol(str, &end, 10);

    assert(end);
    return (errno == 0) && (*end == '\0');
}
#endif

static bool str_eq(char const *const a, char const *const b)
{
    return 0 == strcmp(a, b);
}

static bool str_startswith(char const *const str, char const *const prefix)
{
    return 0 == strncmp(str, prefix, strlen(prefix));
}

static void
set_opt_ptr(struct kcli_option const *const opt, char const *const str)
{
    assert(opt);

    if (opt->ptr_flag)
    {
        *(opt->ptr_flag) = true;
    }

    if (opt->ptr_str)
    {
        *(opt->ptr_str) = str;
    }
}

static bool get_long(
    struct kcli_option const *const opts,
    size_t const count,
    char const *const name,
    struct kcli_option *const out
)
{
    bool ok = false;

    for (size_t i = 0; i < count; ++i)
    {
        struct kcli_option const *const opt = &opts[i];

        if (str_eq(opt->long_name, name))
        {
            *out = *opt;
            ok = true;
            break;
        }
    }

    return ok;
}

static bool get_short(
    struct kcli_option const *const opts,
    size_t const count,
    char const c,
    struct kcli_option *const out
)
{
    bool ok = false;

    for (size_t i = 0; i < count; ++i)
    {
        struct kcli_option const *const opt = &opts[i];

        if (opt->short_name == c)
        {
            *out = *opt;
            ok = true;
            break;
        }
    }

    return ok;
}

static bool get_nth_positional(
    struct kcli_option const *const opts,
    size_t const count,
    size_t n,
    struct kcli_option *const out
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
                *out = *opt;
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

static size_t
get_total_positionals(struct kcli_option const *const opts, size_t const count)
{
    size_t num_pos = 0;

    for (size_t i = 0; i < count; ++i)
    {
        if (opts[i].pos_name)
        {
            ++num_pos;
        }
    }

    return num_pos;
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

        struct kcli_option opt;

        // TODO: '--'

        if (str_startswith(arg, "--"))
        {
            // Long flag

            if (!get_long(opts, count, &arg[2], &opt))
            {
                errorf("Option not found: %s", arg);
                ok = false;
                break;
            }
        }
        else if (arg[0] == '-')
        {
            // Short flag

            char const c = arg[1];

            if (!get_short(opts, count, c, &opt))
            {
                errorf("Option not found: -%c", c);
                ok = false;
                break;
            }
        }
        else
        {
            // Positional arg

            if (!get_nth_positional(opts, count, positional++, &opt))
            {
                error("Too many positional arguments");
                ok = false;
                break;
            }
        }

        set_opt_ptr(&opt, arg);
    }

    if (positional < get_total_positionals(opts, count))
    {
        struct kcli_option opt;
        bool const ok2 = get_nth_positional(opts, count, positional - 1, &opt);
        assert(ok2);

        errorf("Missing argument '%s'", opt.pos_name);
        ok = false;
    }

    return ok;
}
