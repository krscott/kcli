#include "kcli.h" // IWYU pragma: export

#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static bool kcli_str_to_i64(char const *const str, int64_t *const out)
{
    assert(str);
    assert(out);

    char *end = NULL;

    errno = 0;
    *out = strtol(str, &end, 10);

    assert(end);
    return (errno == 0) && (*end == '\0');
}

static bool kcli_str_to_f64(char const *const str, double *const out)
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
    return !opt->short_name && !opt->long_name;
}

static bool kcli_is_flag_opt(struct kcli_option const *const opt)
{
    return opt->short_name != '\0' || opt->long_name != NULL;
}

static bool kcli_is_array_opt(struct kcli_option const *const opt)
{
    return opt->nargs_max >= 1;
}

static void kcli_validate_opts_spec(
    struct kcli_option const *const opts, size_t const count
)
{
    bool positional_array = false;
    bool positional_optional = false;

    for (size_t i = 0; i < count; ++i)
    {
        struct kcli_option const *const opt = &opts[i];

        // Option must have at least one name
        assert(opt->name || opt->short_name || opt->long_name);

        // Option must have at least one ptr_* output
        assert(opt->ptr_flag || opt->ptr_str || opt->ptr_i64 || opt->ptr_f64);

        if (kcli_is_positional_opt(opt))
        {
            // Only the last positional argument may be an array
            assert(!positional_array);
            positional_array = opt->nargs_max > 1;

            // A required positional argument must never follow an optional
            if (positional_optional)
            {
                assert(opt->optional);
            }
            else
            {
                positional_optional = opt->optional;
            }
        }
    }
}

static bool kcli_set_opt_ptr(
    struct kcli_option *const opt,
    char const *const str,
    char *const err,
    size_t const n
)
{
    assert(opt);

    bool ok = true;

    if (opt->_is_used)
    {
        if (kcli_is_positional_opt(opt))
        {
            snprintf(err, n, "Too many uses of '%s'", opt->name);
        }
        else if (opt->long_name)
        {
            snprintf(err, n, "Too many uses of '--%s'", opt->long_name);
        }
        else
        {
            snprintf(err, n, "Too many uses of '-%c'", opt->short_name);
        }

        ok = false;
    }
    else
    {
        if (opt->ptr_flag)
        {
            *(opt->ptr_flag) = true;
        }

        if (opt->ptr_str)
        {
            assert(str);
            *(opt->ptr_str++) = str;
        }

        if (opt->ptr_i64)
        {
            assert(str);
            ok = kcli_str_to_i64(str, opt->ptr_i64++);
            if (!ok)
            {
                snprintf(err, n, "Expected integer, got: %s", str);
            }
        }

        if (opt->ptr_f64)
        {
            assert(str);
            ok = kcli_str_to_f64(str, opt->ptr_f64++);
            if (!ok)
            {
                snprintf(err, n, "Expected number, got: %s", str);
            }
        }

        if (opt->ptr_nargs)
        {
            ++(*opt->ptr_nargs);
        }
        if (opt->nargs_max > 1)
        {
            --(opt->nargs_max);
        }
        else
        {
            opt->_is_used = true;
        }
        opt->_is_partially_used = true;
    }

    return ok;
}

static bool kcli_opt_has_arg(struct kcli_option const *const opt)
{
    return opt->ptr_str || opt->ptr_i64 || opt->ptr_f64;
}

static bool kcli_get_long_opt(
    struct kcli_option *const opts,
    size_t const count,
    char const *const name,
    size_t const size,
    struct kcli_option **const out
)
{
    bool ok = false;

    for (size_t i = 0; i < count; ++i)
    {
        struct kcli_option *const opt = &opts[i];

        if (opt->long_name && (strlen(opt->long_name) == size) &&
            (0 == strncmp(opt->long_name, name, size)))
        {
            *out = opt;
            ok = true;
            break;
        }
    }

    return ok;
}

static bool kcli_get_short_opt(
    struct kcli_option *const opts,
    size_t const count,
    char const c,
    struct kcli_option **const out
)
{
    bool ok = false;

    for (size_t i = 0; i < count; ++i)
    {
        struct kcli_option *const opt = &opts[i];

        if (opt->short_name == c)
        {
            *out = opt;
            ok = true;
            break;
        }
    }

    return ok;
}

static bool kcli_get_next_positional(
    struct kcli_option *const opts,
    size_t const count,
    struct kcli_option **const out
)
{
    bool ok = false;

    for (size_t i = 0; i < count; ++i)
    {
        struct kcli_option *const opt = &opts[i];

        if (kcli_is_positional_opt(opt) && !opt->_is_used)
        {
            if (out)
            {
                *out = opt;
            }
            ok = true;
            break;
        }
    }

    return ok;
}

bool kcli_parse(
    struct kcli_option *const opts,
    size_t const count,
    int const argc,
    char const *const *const argv,
    char *const err_buf,
    size_t const err_buf_size
)
{
    assert(opts);
    assert(argv);
    assert(argc > 0);
    assert(err_buf);

    kcli_validate_opts_spec(opts, count);

    bool ok = true;

    bool double_dash = false;

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

            struct kcli_option *opt;

            if (!kcli_get_long_opt(opts, count, name, name_size, &opt))
            {
                snprintf(err_buf, err_buf_size, "Option not found: %s", arg);
                ok = false;
                goto error;
            }

            if (kcli_opt_has_arg(opt))
            {
                if (split)
                {
                    // Get str after '='
                    ok = kcli_set_opt_ptr(opt, value, err_buf, err_buf_size);
                    if (!ok)
                    {
                        goto error;
                    }
                }
                else
                {
                    if (i + 1 >= argc)
                    {
                        snprintf(
                            err_buf,
                            err_buf_size,
                            "Option requires arg: --%s",
                            opt->long_name
                        );
                        ok = false;
                        goto error;
                    }
                    // Get next full argument in argv
                    ok =
                        kcli_set_opt_ptr(opt, argv[++i], err_buf, err_buf_size);
                    if (!ok)
                    {
                        goto error;
                    }
                }
            }
            else
            {
                ok = kcli_set_opt_ptr(opt, NULL, err_buf, err_buf_size);
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
                struct kcli_option *opt;

                if (!kcli_get_short_opt(opts, count, c, &opt))
                {
                    snprintf(err_buf, err_buf_size, "Option not found: -%c", c);
                    ok = false;
                    goto error;
                }

                if (kcli_opt_has_arg(opt))
                {
                    switch (arg[j + 1])
                    {
                        case '=':
                            // Get str after '='
                            ok = kcli_set_opt_ptr(
                                opt,
                                &arg[j + 2],
                                err_buf,
                                err_buf_size
                            );
                            if (!ok)
                            {
                                goto error;
                            }
                            break;

                        case '\0':
                            if (i + 1 >= argc)
                            {
                                snprintf(
                                    err_buf,
                                    err_buf_size,
                                    "Option requires arg: -%c",
                                    opt->short_name
                                );
                                ok = false;
                                goto error;
                            }

                            // Get next full argument in argv
                            ok = kcli_set_opt_ptr(
                                opt,
                                argv[++i],
                                err_buf,
                                err_buf_size
                            );
                            if (!ok)
                            {
                                goto error;
                            }
                            break;

                        default:
                            // Get str after short flag
                            ok = kcli_set_opt_ptr(
                                opt,
                                &arg[j + 1],
                                err_buf,
                                err_buf_size
                            );
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
                    ok = kcli_set_opt_ptr(opt, NULL, err_buf, err_buf_size);
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
            struct kcli_option *opt;
            ok = kcli_get_next_positional(opts, count, &opt);
            if (!ok)
            {
                snprintf(
                    err_buf,
                    err_buf_size,
                    "Too many positional arguments"
                );
                ok = false;
                goto error;
            }

            ok = kcli_set_opt_ptr(opt, arg, err_buf, err_buf_size);
            if (!ok)
            {
                goto error;
            }
        }
    }

    // Check for missing positional arg

    {
        struct kcli_option *opt;
        if (kcli_get_next_positional(opts, count, &opt) && !opt->optional &&
            !opt->_is_partially_used)
        {
            snprintf(err_buf, err_buf_size, "Missing argument '%s'", opt->name);
            ok = false;
            goto error;
        }
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
                if (kcli_is_array_opt(opt))
                {
                    fprintf(stream, " [%s...]", opt->name);
                }
                else
                {
                    fprintf(stream, " [%s]", opt->name);
                }
            }
            else
            {
                if (kcli_is_array_opt(opt))
                {
                    fprintf(stream, " %s [%s...]", opt->name, opt->name);
                }
                else
                {
                    fprintf(stream, " %s", opt->name);
                }
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

            int col = printf("  %s ", opt->name);

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
                    if (opt->name)
                    {
                        col += printf("=%s", opt->name);
                    }
                    else
                    {
                        col += printf("=ARG");
                    }
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
                    if (opt->name)
                    {
                        col += printf("=%s ", opt->name);
                    }
                    else
                    {
                        col += printf("=ARG ");
                    }
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
