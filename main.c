#include <stdio.h>
#include <stdlib.h>

#include "cutest.h"

static void my_test(CuTestCase *tc)
{
    AssertEqual(tc, 2 + 2, 6);
}

static void my_other_test(CuTestCase *tc)
{
    AssertTrue(tc, 0);
}

int main(void)
{
    CuTestSuite *suite = cu_new_test_suite();

    AddTestCase(suite, my_test);
    AddTestCase(suite, my_other_test);
    cu_run_tests(suite);
    cu_print_results(suite, stderr);

    return EXIT_SUCCESS;
}
