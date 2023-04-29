#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "cutest.h"

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
};

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
    CuAssertArg *arg1, CuAssertArg *arg2,
    char const *file, int line)
{
    int result;

    switch (type)
    {
    case CU_ASSERT_EQUAL:
        result = arg1->value.i == arg2->value.i;
        break;
    case CU_ASSERT_TRUE:
        result = arg1->value.i;
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
    }

    return result;
}

void cu_run_tests(CuTestSuite *suite)
{
    CuTestCase *tc;

    for (tc = suite->head; tc != NULL; tc = tc->next)
    {
        tc->func(tc);
    }

    suite->completed = 1;
}

void cu_print_results(CuTestSuite *suite, FILE *out)
{
    if (suite->completed)
    {
        CuTestCase *tc;

        for (tc = suite->head; tc != NULL; tc = tc->next)
        {
            if (tc->failed)
            {
                struct fail_info const *info = &tc->fail_info;

                fprintf(out, "FAIL: %s, %s:%d\n", tc->name, info->file, info->line);
                switch (info->type)
                {
                case CU_ASSERT_EQUAL:
                    fprintf(out, "\tAssertEqual(%s, %s)\n", info->arg1.expr, info->arg2.expr);
                    fprintf(out, "\t%d != %d\n", info->arg1.value.i, info->arg2.value.i);
                    break;
                case CU_ASSERT_TRUE:
                    fprintf(out, "\tAssertTrue(%s)\n", info->arg1.expr);
                    fprintf(out, "\t%d is not true\n", info->arg1.value.i);
                    break;
                default:
                    break;
                }
            }
        }
    }
}
