#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include "cutest.h"

static void my_test(CuTestCase *tc)
{
    AssertEqual(tc, 2 + 2, 4);
    AssertNotEqual(tc, -1, UINT_MAX);
}

static void my_other_test(CuTestCase *tc)
{
    AssertTrue(tc, !0);
    AssertFalse(tc, 100);
}

static void test_null(CuTestCase *tc)
{
    AssertNull(tc, tc);
    AssertNotNull(tc, tc);
}

static void test_str(CuTestCase *tc)
{
    char fish[] = "fish";

    fish[0] = 'w';
    AssertStrEqual(tc, fish, "fish");
}

int main(void)
{
    CuTestSuite *suite = cu_new_test_suite();

    AddTestCase(suite, my_test);
    AddTestCase(suite, my_other_test);
    AddTestCase(suite, test_null);
    AddTestCase(suite, test_str);

    cu_run_tests(suite, stderr);
    cu_print_results(suite, stderr);

    return EXIT_SUCCESS;
}
