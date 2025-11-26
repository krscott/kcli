#include "kclilib.h"

#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define error(msg) printf("CLI Error: " msg "\n")
#define errorf(fmt, ...) printf("CLI Error: " fmt "\n", __VA_ARGS__)

static bool str_to_long(char const *const str, long *const out)
{
    assert(str);
    assert(out);

    char *end = NULL;

    errno = 0;
    *out = strtol(str, &end, 10);

    assert(end);
    return (errno == 0) && (*end == '\0');
}

static bool str_to_double(char const *const str, double *const out)
{
    assert(str);
    assert(out);

    char *end = NULL;

    errno = 0;
    *out = strtod(str, &end);

    assert(end);
    return (errno == 0) && (*end == '\0');
}

static bool str_startswith(char const *const str, char const *const prefix)
{
    return 0 == strncmp(str, prefix, strlen(prefix));
}

static bool str_split(
    char const *str,
    char const c,
    size_t *const head_size,
    char const **const tail
)
{
    assert(c != '\0');

    bool found = false;
    size_t i = 0;

    for (; str[i] && str[i] != c; ++i)
    {
    }

    if (str[i] == c)
    {
        if (tail)
        {
            *tail = &str[i + 1];
        }
        found = true;
    }
    else if (tail)
    {
        *tail = NULL;
    }

    if (head_size)
    {
        *head_size = i;
    }

    return found;
}

static bool is_positional_opt(struct kcli_option const *const opt)
{
    return opt->pos_name != NULL;
}

static bool is_flag_opt(struct kcli_option const *const opt)
{
    return opt->short_name != '\0' || opt->long_name != NULL;
}

static void
validate_opts_spec(struct kcli_option const *const opts, size_t const count)
{
    for (size_t i = 0; i < count; ++i)
    {
        struct kcli_option const *const opt = &opts[i];

        // Option must have at least one ptr_* output
        assert(
            opt->ptr_flag || opt->ptr_str || opt->ptr_long || opt->ptr_double
        );
    }
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
        assert(str);
        *(opt->ptr_str) = str;
    }

    if (opt->ptr_long)
    {
        assert(str);
        str_to_long(str, opt->ptr_long);
    }

    if (opt->ptr_double)
    {
        assert(str);
        str_to_double(str, opt->ptr_double);
    }
}

static bool needs_arg(struct kcli_option const *const opt)
{
    return opt->ptr_str || opt->ptr_long || opt->ptr_double;
}

static bool get_long(
    struct kcli_option const *const opts,
    size_t const count,
    char const *const name,
    size_t const size,
    struct kcli_option *const out
)
{
    bool ok = false;

    for (size_t i = 0; i < count; ++i)
    {
        struct kcli_option const *const opt = &opts[i];

        if (opt->long_name && (strlen(opt->long_name) == size) &&
            (0 == strncmp(opt->long_name, name, size)))
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
get_min_positionals(struct kcli_option const *const opts, size_t const count)
{
    size_t num_pos = 0;

    for (size_t i = 0; i < count; ++i)
    {
        if (opts[i].pos_name)
        {
            if (opts[i].optional)
            {
                break;
            }

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

    validate_opts_spec(opts, count);

    bool ok = true;

    bool double_dash = false;
    size_t positional = 0;

    for (int i = 0; i < argc; ++i)
    {
        char const *const arg = argv[i];

        if (!double_dash && 0 == strcmp(arg, "--"))
        {
            double_dash = true;
        }
        else if (!double_dash && str_startswith(arg, "--"))
        {
            // Long flag
            char const *name = &arg[2];

            char const *value;
            size_t name_size;
            bool const split = str_split(name, '=', &name_size, &value);

            struct kcli_option opt;

            if (!get_long(opts, count, name, name_size, &opt))
            {
                errorf("Option not found: %s", arg);
                ok = false;
                goto error;
            }

            if (needs_arg(&opt))
            {
                if (split)
                {
                    // Get str after '='
                    set_opt_ptr(&opt, value);
                }
                else
                {
                    // Get next full argument in argv
                    set_opt_ptr(&opt, argv[++i]);
                }
            }
            else
            {
                set_opt_ptr(&opt, NULL);
            }
        }
        else if (!double_dash && arg[0] == '-')
        {
            // Short flag(s)

            for (size_t j = 1; arg[j]; ++j)
            {
                char const c = arg[j];
                struct kcli_option opt;

                if (!get_short(opts, count, c, &opt))
                {
                    errorf("Option not found: -%c", c);
                    ok = false;
                    goto error;
                }

                if (needs_arg(&opt))
                {
                    switch (arg[j + 1])
                    {
                        case '=':
                            // Get str after '='
                            set_opt_ptr(&opt, &arg[j + 2]);
                            break;

                        case '\0':
                            // Get next full argument in argv
                            set_opt_ptr(&opt, argv[++i]);
                            break;

                        default:
                            // Get str after short flag
                            set_opt_ptr(&opt, &arg[j + 1]);
                            break;
                    }

                    break;
                }
                else
                {
                    set_opt_ptr(&opt, NULL);
                }
            }
        }
        else
        {
            // Positional arg
            struct kcli_option opt;

            if (!get_nth_positional(opts, count, positional++, &opt))
            {
                error("Too many positional arguments");
                ok = false;
                goto error;
            }

            set_opt_ptr(&opt, arg);
        }
    }

    if (positional < get_min_positionals(opts, count))
    {
        struct kcli_option opt;
        bool const ok2 = get_nth_positional(opts, count, positional - 1, &opt);
        assert(ok2);

        errorf("Missing argument '%s'", opt.pos_name);
        ok = false;
    }

error:
    return ok;
}

static void print_align(int col)
{
    int const HELP_INDENT = 20;

    // Check if align should be put on new line
    if (col > HELP_INDENT)
    {
        putchar('\n');
        col = 0;
    }

    while (col < HELP_INDENT)
    {
        putchar(' ');
        ++col;
    }
}

void kcli_print_usage(
    char const *const prog_name,
    struct kcli_option const *const opts,
    size_t const count
)
{
    printf("Usage: %s", prog_name);

    for (size_t i = 0; i < count; ++i)
    {
        struct kcli_option const *const opt = &opts[i];
        if (is_flag_opt(opt))
        {
            printf(" [opts]");
            break;
        }
    }

    for (size_t i = 0; i < count; ++i)
    {
        struct kcli_option const *const opt = &opts[i];
        if (is_positional_opt(opt))
        {
            if (opt->optional)
            {
                printf(" [%s]", opt->pos_name);
            }
            else
            {
                printf(" %s", opt->pos_name);
            }
        }
    }

    printf("\n");
}

void kcli_print_help(
    char const *const prog_name,
    struct kcli_option const *const opts,
    size_t const count
)
{
    kcli_print_usage(prog_name, opts, count);

    bool any_positional = false;
    for (size_t i = 0; i < count; ++i)
    {
        struct kcli_option const *const opt = &opts[i];
        if (is_positional_opt(opt))
        {
            if (!any_positional)
            {
                any_positional = true;
                printf("\nParameters:\n");
            }

            int col = printf("  %s ", opt->pos_name);

            if (opt->help)
            {
                print_align(col);
                printf("%s", opt->help);
            }

            printf("\n");
        }
    }

    bool any_flag = false;
    for (size_t i = 0; i < count; ++i)
    {
        struct kcli_option const *const opt = &opts[i];
        if (is_flag_opt(opt))
        {
            if (!any_flag)
            {
                any_flag = true;
                printf("\nOptions:\n");
            }

            int col = printf("  ");

            if (opt->short_name)
            {
                col += printf("-%c", opt->short_name);

                if (needs_arg(opt))
                {
                    col += printf("=ARG");
                }
            }

            if (opt->short_name && opt->long_name)
            {
                col += printf(", ");
            }
            else
            {
                col += printf(" ");
            }

            if (opt->long_name)
            {
                col += printf("--%s", opt->long_name);

                if (needs_arg(opt))
                {
                    col += printf("=ARG ");
                }
                else
                {
                    col += printf(" ");
                }
            }

            if (opt->help)
            {
                print_align(col);
                printf("%s", opt->help);
            }

            printf("\n");
        }
    }
}
