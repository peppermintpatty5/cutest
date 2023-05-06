#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/time.h>
#include <unistd.h>

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

static void test_sleep(CuTestCase *tc)
{
    (void)tc;
    sleep(1);
}

static float timer(int reset)
{
    static struct timeval start, stop;

    if (reset)
    {
        gettimeofday(&start, NULL);
        return 0.0f;
    }
    else
    {
        gettimeofday(&stop, NULL);
        return (stop.tv_sec - start.tv_sec) +
               (stop.tv_usec - start.tv_usec) * 1e-6f;
    }
}

int main(void)
{
    CuTestSuite *suite = NewTestSuite();

    AddTestCase(suite, my_test);
    AddTestCase(suite, my_other_test);
    AddTestCase(suite, test_null);
    AddTestCase(suite, test_str);
    AddTestCase(suite, test_sleep);

    cu_run_tests(suite, stderr, timer);
    cu_print_results(suite, stderr);

    return EXIT_SUCCESS;
}
