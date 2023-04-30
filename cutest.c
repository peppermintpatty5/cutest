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

int cu_do_assertion(
    CuTestCase *tc, CuAssertType type,
    CuAssertArg const *arg1, CuAssertArg const *arg2,
    char const *file, int line)
{
    int result;

    switch (type)
    {
    case CU_ASSERT_EQUAL:
        result = arg1->value.i == arg2->value.i;
        break;
    case CU_ASSERT_NOT_EQUAL:
        result = arg1->value.i != arg2->value.i;
        break;
    case CU_ASSERT_STR_EQUAL:
        result = strcmp(arg1->value.s, arg2->value.s) == 0;
        break;
    case CU_ASSERT_STR_NOT_EQUAL:
        result = strcmp(arg1->value.s, arg2->value.s) != 0;
        break;
    case CU_ASSERT_TRUE:
        result = arg1->value.i;
        break;
    case CU_ASSERT_FALSE:
        result = !arg1->value.i;
        break;
    case CU_ASSERT_NULL:
        result = arg1->value.p == NULL;
        break;
    case CU_ASSERT_NOT_NULL:
        result = arg1->value.p != NULL;
        break;
    default:
        break;
    }

    if (!result)
    {
        struct fail_info *info = &tc->fail_info;

        info->arg1 = *arg1;
        info->arg2 = *arg2;
        info->file = file;
        info->line = line;
        info->type = type;
        tc->failed = 1;

        /* string values must be copied */
        if (type == CU_ASSERT_STR_EQUAL || type == CU_ASSERT_STR_NOT_EQUAL)
        {
            char *str1, *str2;

            str1 = malloc(strlen(arg1->value.s) + 1);
            str2 = malloc(strlen(arg2->value.s) + 1);

            info->arg1.value.s = strcpy(str1, arg1->value.s);
            info->arg2.value.s = strcpy(str2, arg2->value.s);
        }
    }

    return result;
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
                case CU_ASSERT_EQUAL:
                case CU_ASSERT_NOT_EQUAL:
                case CU_ASSERT_STR_EQUAL:
                case CU_ASSERT_STR_NOT_EQUAL:
                    fprintf(out, "Assert%s(%s, %s)\n",
                            suffix, arg1->expr, arg2->expr);
                    break;
                case CU_ASSERT_TRUE:
                case CU_ASSERT_FALSE:
                case CU_ASSERT_NULL:
                case CU_ASSERT_NOT_NULL:
                    fprintf(out, "Assert%s(%s)\n",
                            suffix, arg1->expr);
                    break;
                default:
                    break;
                }

                fprintf(out, "Error:\n\t");
                switch (info->type)
                {
                case CU_ASSERT_EQUAL:
                    fprintf(out, "%d != %d\n", arg1->value.i, arg2->value.i);
                    break;
                case CU_ASSERT_NOT_EQUAL:
                    fprintf(out, "%d == %d\n", arg1->value.i, arg2->value.i);
                    break;
                case CU_ASSERT_STR_EQUAL:
                    fprintf(out, "%s != %s\n", arg1->value.s, arg2->value.s);
                    break;
                case CU_ASSERT_STR_NOT_EQUAL:
                    fprintf(out, "%s == %s\n", arg1->value.s, arg2->value.s);
                    break;
                case CU_ASSERT_TRUE:
                    fprintf(out, "%d is not true\n", arg1->value.i);
                    break;
                case CU_ASSERT_FALSE:
                    fprintf(out, "%d is not false\n", arg1->value.i);
                    break;
                case CU_ASSERT_NULL:
                    fprintf(out, "%p is not null\n", arg1->value.p);
                    break;
                case CU_ASSERT_NOT_NULL:
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
