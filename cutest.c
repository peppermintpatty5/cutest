#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "cutest.h"

/**
 * Maps cu_assert_type to strings.
 */
static char const *const assert_type_str[] = {
    "Equal",
    "True"};

struct cu_test_case
{
    CuTestCase *next;
    void (*func)(CuTestCase *);
    char const *name;
    struct fail_info
    {
        char const *arg1;
        char const *arg2;
        char const *file;
        int line;
        enum cu_assert_type type;
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

void cu_set_failed(
    CuTestCase *tc, enum cu_assert_type type,
    char const *file, int line,
    char const *arg1, char const *arg2)
{
    struct fail_info *info = &tc->fail_info;

    info->arg1 = arg1;
    info->arg2 = arg2;
    info->file = file;
    info->line = line;
    info->type = type;
    tc->failed = 1;
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
                char const *suffix = assert_type_str[info->type];

                fprintf(out, "FAIL: %s\n", tc->name);
                if (info->arg2 != NULL)
                    fprintf(out, "\tAssert%s(%s, %s)\n", suffix, info->arg1, info->arg2);
                else
                    fprintf(out, "\tAssert%s(%s)\n", suffix, info->arg1);
            }
        }
    }
}
