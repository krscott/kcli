#include "kcli.h"

#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static bool kcli_str_to_long(char const *const str, long *const out)
{
    assert(str);
    assert(out);

    char *end = NULL;

    errno = 0;
    *out = strtol(str, &end, 10);

    assert(end);
    return (errno == 0) && (*end == '\0');
}

static bool kcli_str_to_double(char const *const str, double *const out)
{
    assert(str);
    assert(out);

    char *end = NULL;

    errno = 0;
    *out = strtod(str, &end);

    assert(end);
    return (errno == 0) && (*end == '\0');
}

static bool kcli_str_startswith(char const *const str, char const *const prefix)
{
    return 0 == strncmp(str, prefix, strlen(prefix));
}

static bool kcli_str_split(
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

static bool kcli_is_positional_opt(struct kcli_option const *const opt)
{
    return opt->pos_name != NULL;
}

static bool kcli_is_flag_opt(struct kcli_option const *const opt)
{
    return opt->short_name != '\0' || opt->long_name != NULL;
}

static void kcli_validate_opts_spec(
    struct kcli_option const *const opts, size_t const count
)
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

static bool
kcli_set_opt_ptr(struct kcli_option const *const opt, char const *const str)
{
    assert(opt);

    bool ok = true;

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
        ok = kcli_str_to_long(str, opt->ptr_long);
        if (!ok)
        {
            kcli_errorf("Expected integer, got: %s", str);
        }
    }

    if (opt->ptr_double)
    {
        assert(str);
        ok = kcli_str_to_double(str, opt->ptr_double);
        if (!ok)
        {
            kcli_errorf("Expected number, got: %s", str);
        }
    }

    return ok;
}

static bool kcli_opt_has_arg(struct kcli_option const *const opt)
{
    return opt->ptr_str || opt->ptr_long || opt->ptr_double;
}

static bool kcli_get_long_opt(
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

static bool kcli_get_short_opt(
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

static bool kcli_get_nth_positional(
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

static size_t kcli_get_min_positionals(
    struct kcli_option const *const opts, size_t const count
)
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
    assert(argc > 0);

    kcli_validate_opts_spec(opts, count);

    bool ok = true;

    bool double_dash = false;
    size_t positional = 0;

    for (int i = 1; i < argc; ++i)
    {
        char const *const arg = argv[i];

        if (!double_dash && 0 == strcmp(arg, "--"))
        {
            double_dash = true;
        }
        else if (!double_dash && kcli_str_startswith(arg, "--"))
        {
            // Long flag
            char const *name = &arg[2];

            char const *value;
            size_t name_size;
            bool const split = kcli_str_split(name, '=', &name_size, &value);

            struct kcli_option opt;

            if (!kcli_get_long_opt(opts, count, name, name_size, &opt))
            {
                kcli_errorf("Option not found: %s", arg);
                ok = false;
                goto error;
            }

            if (kcli_opt_has_arg(&opt))
            {
                if (split)
                {
                    // Get str after '='
                    ok = kcli_set_opt_ptr(&opt, value);
                    if (!ok)
                    {
                        goto error;
                    }
                }
                else
                {
                    if (i + 1 >= argc)
                    {
                        kcli_errorf("Option requires arg: --%s", opt.long_name);
                        ok = false;
                        goto error;
                    }
                    // Get next full argument in argv
                    ok = kcli_set_opt_ptr(&opt, argv[++i]);
                    if (!ok)
                    {
                        goto error;
                    }
                }
            }
            else
            {
                ok = kcli_set_opt_ptr(&opt, NULL);
                if (!ok)
                {
                    goto error;
                }
            }
        }
        else if (!double_dash && arg[0] == '-')
        {
            // Short flag(s)

            for (size_t j = 1; arg[j]; ++j)
            {
                char const c = arg[j];
                struct kcli_option opt;

                if (!kcli_get_short_opt(opts, count, c, &opt))
                {
                    kcli_errorf("Option not found: -%c", c);
                    ok = false;
                    goto error;
                }

                if (kcli_opt_has_arg(&opt))
                {
                    switch (arg[j + 1])
                    {
                        case '=':
                            // Get str after '='
                            ok = kcli_set_opt_ptr(&opt, &arg[j + 2]);
                            if (!ok)
                            {
                                goto error;
                            }
                            break;

                        case '\0':
                            if (i + 1 >= argc)
                            {
                                kcli_errorf(
                                    "Option requires arg: -%c",
                                    opt.short_name
                                );
                                ok = false;
                                goto error;
                            }

                            // Get next full argument in argv
                            ok = kcli_set_opt_ptr(&opt, argv[++i]);
                            if (!ok)
                            {
                                goto error;
                            }
                            break;

                        default:
                            // Get str after short flag
                            ok = kcli_set_opt_ptr(&opt, &arg[j + 1]);
                            if (!ok)
                            {
                                goto error;
                            }
                            break;
                    }

                    break;
                }
                else
                {
                    ok = kcli_set_opt_ptr(&opt, NULL);
                    if (!ok)
                    {
                        goto error;
                    }
                }
            }
        }
        else
        {
            // Positional arg
            struct kcli_option opt;

            if (!kcli_get_nth_positional(opts, count, positional++, &opt))
            {
                kcli_error("Too many positional arguments");
                ok = false;
                goto error;
            }

            ok = kcli_set_opt_ptr(&opt, arg);
            if (!ok)
            {
                goto error;
            }
        }
    }

    if (positional < kcli_get_min_positionals(opts, count))
    {
        struct kcli_option opt;
        bool const ok2 = kcli_get_nth_positional(opts, count, positional, &opt);
        assert(ok2);

        kcli_errorf("Missing argument '%s'", opt.pos_name);
        ok = false;
        goto error;
    }

error:
    return ok;
}

static void kcli_print_align(int col)
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
    FILE *const stream,
    char const *const prog_name,
    struct kcli_option const *const opts,
    size_t const count
)
{
    fprintf(stream, "Usage: %s", prog_name);

    for (size_t i = 0; i < count; ++i)
    {
        struct kcli_option const *const opt = &opts[i];
        if (kcli_is_flag_opt(opt))
        {
            fprintf(stream, " [opts]");
            break;
        }
    }

    for (size_t i = 0; i < count; ++i)
    {
        struct kcli_option const *const opt = &opts[i];
        if (kcli_is_positional_opt(opt))
        {
            if (opt->optional)
            {
                fprintf(stream, " [%s]", opt->pos_name);
            }
            else
            {
                fprintf(stream, " %s", opt->pos_name);
            }
        }
    }

    fprintf(stream, "\n");
}

void kcli_print_help(
    char const *const prog_name,
    struct kcli_option const *const opts,
    size_t const count
)
{
    kcli_print_usage(stdout, prog_name, opts, count);

    bool any_positional = false;
    for (size_t i = 0; i < count; ++i)
    {
        struct kcli_option const *const opt = &opts[i];
        if (kcli_is_positional_opt(opt))
        {
            if (!any_positional)
            {
                any_positional = true;
                printf("\nParameters:\n");
            }

            int col = printf("  %s ", opt->pos_name);

            if (opt->help)
            {
                kcli_print_align(col);
                printf("%s", opt->help);
            }

            printf("\n");
        }
    }

    bool any_flag = false;
    for (size_t i = 0; i < count; ++i)
    {
        struct kcli_option const *const opt = &opts[i];
        if (kcli_is_flag_opt(opt))
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

                if (kcli_opt_has_arg(opt))
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

                if (kcli_opt_has_arg(opt))
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
                kcli_print_align(col);
                printf("%s", opt->help);
            }

            printf("\n");
        }
    }
}
