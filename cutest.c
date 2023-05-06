#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cutest.h"

#define MAX_ASSERT_ARGS 2
#define RESULTS_BORDER_LENGTH 70U

/**
 * Maps CuAssertType to corresponding macro suffix string.
 */
static char const *const assert_type_suffixes[] = {
    "Equal",
    "NotEqual",
    "StrEqual",
    "StrNotEqual",
    "True",
    "False",
    "Null",
    "NotNull"};

/**
 * Represents an argument passed to an assertion macro.
 */
struct assert_arg
{
    char const *expr; /* Argument's original string representation */
    union             /* Argument's actual value */
    {
        void const *p;
        char const *s;
        int i;
    } value;
};

/**
 * All possible assertion types.
 */
enum assert_type
{
    ASSERT_EQUAL,
    ASSERT_NOT_EQUAL,
    ASSERT_STR_EQUAL,
    ASSERT_STR_NOT_EQUAL,
    ASSERT_TRUE,
    ASSERT_FALSE,
    ASSERT_NULL,
    ASSERT_NOT_NULL
};

/**
 * Records information about a failed assertion.
 */
struct fail_info
{
    struct assert_arg args[MAX_ASSERT_ARGS]; /* Arguments passed to assertion */
    char const *file;                        /* Name of file */
    int line;                                /* Line number */
    enum assert_type type;                   /* Type of assertion */
};

struct cu_test_case
{
    CuTestCase *next;           /* Next test case */
    void (*func)(CuTestCase *); /* Function associated with this test case */
    char const *name;           /* Name of function */
    struct fail_info fail_info; /* Failure information (only if failed) */
    int failed;                 /* Non-zero if failure occurred */
};

struct cu_test_suite
{
    CuTestCase *head;   /* First test case */
    CuTestCase *tail;   /* Last test case */
    unsigned num_cases; /* Number of test cases */
    int completed;      /* Non-zero if ran at least once */
    float time_elapsed; /* Time elapsed of last recorded run */
};

/**
 * Print character to output stream repeatedly followed by a newline.
 */
static void print_row(FILE *out, char c, unsigned n)
{
    unsigned i;

    for (i = 0; i < n; i++)
        fputc(c, out);
    fputc('\n', out);
}

CuTestSuite *cu_new_test_suite(void)
{
    CuTestSuite *suite = malloc(sizeof(CuTestSuite));

    suite->head = NULL;
    suite->tail = NULL;
    suite->num_cases = 0;
    suite->completed = 0;
    suite->time_elapsed = 0.0f;

    return suite;
}

void cu_add_test_case(
    CuTestSuite *suite, void (*func)(CuTestCase *), char const *name)
{
    CuTestCase *tc = malloc(sizeof(CuTestCase));

    tc->next = NULL;
    tc->func = func;
    tc->name = name;
    tc->failed = 0;

    if (suite->tail != NULL)
        suite->tail->next = tc;
    else
        suite->head = tc;

    suite->tail = tc;
}

void cu_run_tests(CuTestSuite *suite, FILE *out, timer_fn_t timer)
{
    CuTestCase *tc;

    /* start timer */
    if (timer != NULL)
        timer(1);

    for (tc = suite->head; tc != NULL; tc = tc->next)
    {
        tc->func(tc);
        if (out != NULL)
            fputc(tc->failed ? 'F' : '.', out);
    }
    if (out != NULL)
        fputc('\n', out);

    suite->time_elapsed = timer != NULL ? timer(0) : 0.0f;
    suite->completed = 1;
}

void cu_print_results(CuTestSuite *suite, FILE *out)
{
    if (suite->completed)
    {
        CuTestCase *tc;
        unsigned failures = 0;

        for (tc = suite->head; tc != NULL; tc = tc->next)
        {
            if (tc->failed)
            {
                struct fail_info const *info = &tc->fail_info;
                struct assert_arg const *arg1 = &info->args[0],
                                        *arg2 = &info->args[1];
                char const *suffix = assert_type_suffixes[info->type];

                failures++;

                print_row(out, '=', RESULTS_BORDER_LENGTH);
                fprintf(out, "FAIL: %s\n", tc->name);
                print_row(out, '-', RESULTS_BORDER_LENGTH);

                fprintf(out, "%s:%d:\n\t", info->file, info->line);
                switch (info->type)
                {
                case ASSERT_EQUAL:
                case ASSERT_NOT_EQUAL:
                case ASSERT_STR_EQUAL:
                case ASSERT_STR_NOT_EQUAL:
                    fprintf(out, "Assert%s(%s, %s)\n",
                            suffix, arg1->expr, arg2->expr);
                    break;
                case ASSERT_TRUE:
                case ASSERT_FALSE:
                case ASSERT_NULL:
                case ASSERT_NOT_NULL:
                    fprintf(out, "Assert%s(%s)\n",
                            suffix, arg1->expr);
                    break;
                default:
                    break;
                }

                fprintf(out, "Error:\n\t");
                switch (info->type)
                {
                case ASSERT_EQUAL:
                    fprintf(out, "%d != %d\n", arg1->value.i, arg2->value.i);
                    break;
                case ASSERT_NOT_EQUAL:
                    fprintf(out, "%d == %d\n", arg1->value.i, arg2->value.i);
                    break;
                case ASSERT_STR_EQUAL:
                    fprintf(out, "%s != %s\n", arg1->value.s, arg2->value.s);
                    break;
                case ASSERT_STR_NOT_EQUAL:
                    fprintf(out, "%s == %s\n", arg1->value.s, arg2->value.s);
                    break;
                case ASSERT_TRUE:
                    fprintf(out, "%d is not true\n", arg1->value.i);
                    break;
                case ASSERT_FALSE:
                    fprintf(out, "%d is not false\n", arg1->value.i);
                    break;
                case ASSERT_NULL:
                    fprintf(out, "%p is not null\n", arg1->value.p);
                    break;
                case ASSERT_NOT_NULL:
                    fprintf(out, "%p is null\n", arg1->value.p);
                    break;
                default:
                    break;
                }

                fputc('\n', out);
            }
        }

        print_row(out, '-', RESULTS_BORDER_LENGTH);
        fprintf(out, "Ran %u test%s in %.3fs\n\n",
                suite->num_cases,
                suite->num_cases != 1 ? "s" : "",
                suite->time_elapsed);
        if (failures > 0)
            fprintf(out, "FAILED (failures=%u)\n", failures);
        else
            fprintf(out, "OK\n");
    }
}

int cu_assert_equal(CuTestCase *tc, int i1, int i2, int negate)
{
    int result = negate ? i1 != i2 : i1 == i2;

    if (!result)
    {
        tc->fail_info.args[0].value.i = i1;
        tc->fail_info.args[1].value.i = i2;
        tc->fail_info.type = negate ? ASSERT_NOT_EQUAL : ASSERT_EQUAL;
    }

    return result;
}

int cu_assert_str_equal(
    CuTestCase *tc, char const *s1, char const *s2, int negate)
{
    int result = negate ? strcmp(s1, s2) != 0 : strcmp(s1, s2) == 0;

    if (!result)
    {
        tc->fail_info.args[0].value.s = strcpy(malloc(strlen(s1) + 1), s1);
        tc->fail_info.args[1].value.s = strcpy(malloc(strlen(s2) + 1), s2);
        tc->fail_info.type = negate ? ASSERT_STR_NOT_EQUAL : ASSERT_STR_EQUAL;
    }

    return result;
}

int cu_assert_true(CuTestCase *tc, int b, int negate)
{
    int result = negate ? !b : b;

    if (!result)
    {
        tc->fail_info.args[0].value.i = b;
        tc->fail_info.type = negate ? ASSERT_FALSE : ASSERT_TRUE;
    }

    return result;
}

int cu_assert_null(CuTestCase *tc, void const *p, int negate)
{
    int result = negate ? p != NULL : p == NULL;

    if (!result)
    {
        tc->fail_info.args[0].value.p = p;
        tc->fail_info.type = negate ? ASSERT_NOT_NULL : ASSERT_NULL;
    }

    return result;
}

void cu_set_failed(
    CuTestCase *tc, char const *file, int line,
    char const *expr1, char const *expr2)
{
    tc->fail_info.args[0].expr = expr1;
    tc->fail_info.args[1].expr = expr2;
    tc->fail_info.file = file;
    tc->fail_info.line = line;
    tc->failed = 1;
}
