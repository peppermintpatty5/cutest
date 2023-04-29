#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cutest.h"

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
                char const *suffix = assert_type_suffixes[info->type];
                CuAssertArg const *arg1 = &info->arg1,
                                  *arg2 = &info->arg2;

                fprintf(out, "FAIL: %s, %s:%d\n", tc->name, info->file, info->line);

                switch (info->type)
                {
                case CU_ASSERT_EQUAL:
                case CU_ASSERT_NOT_EQUAL:
                case CU_ASSERT_STR_EQUAL:
                case CU_ASSERT_STR_NOT_EQUAL:
                    fprintf(out, "\tAssert%s(%s, %s)\n", suffix, arg1->expr, arg2->expr);
                    break;
                case CU_ASSERT_TRUE:
                case CU_ASSERT_FALSE:
                case CU_ASSERT_NULL:
                case CU_ASSERT_NOT_NULL:
                    fprintf(out, "\tAssert%s(%s)\n", suffix, arg1->expr);
                    break;
                default:
                    break;
                }

                switch (info->type)
                {
                case CU_ASSERT_EQUAL:
                    fprintf(out, "\t%d != %d\n", arg1->value.i, arg2->value.i);
                    break;
                case CU_ASSERT_NOT_EQUAL:
                    fprintf(out, "\t%d == %d\n", arg1->value.i, arg2->value.i);
                    break;
                case CU_ASSERT_STR_EQUAL:
                    fprintf(out, "\t%s != %s\n", arg1->value.s, arg2->value.s);
                    break;
                case CU_ASSERT_STR_NOT_EQUAL:
                    fprintf(out, "\t%s == %s\n", arg1->value.s, arg2->value.s);
                    break;
                case CU_ASSERT_TRUE:
                    fprintf(out, "\t%d is not true\n", arg1->value.i);
                    break;
                case CU_ASSERT_FALSE:
                    fprintf(out, "\t%d is not false\n", arg1->value.i);
                    break;
                case CU_ASSERT_NULL:
                    fprintf(out, "\t%p is not null\n", arg1->value.p);
                    break;
                case CU_ASSERT_NOT_NULL:
                    fprintf(out, "\t%p is null\n", arg1->value.p);
                    break;
                default:
                    break;
                }
            }
        }
    }
}
