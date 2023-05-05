#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cutest.h"

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
typedef struct
{
    /* The argument's original string representation */
    char const *expr;
    /* The argument's actual value */
    union cu_value
    {
        void const *p;
        char const *s;
        int i;
    } value;
} CuAssertArg;

/**
 * All possible assertion types.
 */
typedef enum
{
    ASSERT_EQUAL,
    ASSERT_NOT_EQUAL,
    ASSERT_STR_EQUAL,
    ASSERT_STR_NOT_EQUAL,
    ASSERT_TRUE,
    ASSERT_FALSE,
    ASSERT_NULL,
    ASSERT_NOT_NULL
} CuAssertType;

struct cu_test_case
{
    CuTestCase *next;
    void (*func)(CuTestCase *);
    char const *name;
    struct fail_info
    {
        CuAssertArg arg1, arg2;
        char const *file;
        int line;
        CuAssertType type;
    } fail_info;
    int failed;
};

struct cu_test_suite
{
    CuTestCase *head;
    CuTestCase *tail;
    int completed;
    float time_elapsed;
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

    return suite;
}

CuTestCase *cu_new_test_case(void (*func)(CuTestCase *), char const *name)
{
    CuTestCase *tc = malloc(sizeof(CuTestCase));

    tc->next = NULL;
    tc->func = func;
    tc->name = name;
    tc->failed = 0;

    return tc;
}

void cu_add_test(CuTestSuite *suite, CuTestCase *tc)
{
    assert(suite != NULL);
    assert(tc != NULL);

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
        unsigned num_tests = 0, failures = 0;

        for (tc = suite->head; tc != NULL; tc = tc->next)
        {
            num_tests++;

            if (tc->failed)
            {
                struct fail_info const *info = &tc->fail_info;
                char const *suffix = assert_type_suffixes[info->type];
                CuAssertArg const *arg1 = &info->arg1,
                                  *arg2 = &info->arg2;

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
                num_tests, num_tests != 1 ? "s" : "", suite->time_elapsed);
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
        tc->fail_info.arg1.value.i = i1;
        tc->fail_info.arg2.value.i = i2;
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
        tc->fail_info.arg1.value.s = strcpy(malloc(strlen(s1) + 1), s1);
        tc->fail_info.arg2.value.s = strcpy(malloc(strlen(s2) + 1), s2);
        tc->fail_info.type = negate ? ASSERT_STR_NOT_EQUAL : ASSERT_STR_EQUAL;
    }

    return result;
}

int cu_assert_true(CuTestCase *tc, int b, int negate)
{
    int result = negate ? !b : b;

    if (!result)
    {
        tc->fail_info.arg1.value.i = b;
        tc->fail_info.type = negate ? ASSERT_FALSE : ASSERT_TRUE;
    }

    return result;
}

int cu_assert_null(CuTestCase *tc, void const *p, int negate)
{
    int result = negate ? p != NULL : p == NULL;

    if (!result)
    {
        tc->fail_info.arg1.value.p = p;
        tc->fail_info.type = negate ? ASSERT_NOT_NULL : ASSERT_NULL;
    }

    return result;
}

void cu_set_failed(
    CuTestCase *tc, char const *file, int line,
    char const *expr1, char const *expr2)
{
    tc->fail_info.arg1.expr = expr1;
    tc->fail_info.arg2.expr = expr2;
    tc->fail_info.file = file;
    tc->fail_info.line = line;
    tc->failed = 1;
}
