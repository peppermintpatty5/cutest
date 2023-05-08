#ifndef CUTEST_H_
#define CUTEST_H_

#include <stdio.h>

/**
 * Create a new test suite.
 */
#define NewTestSuite() cu_new_test_suite()

/**
 * Add a test case to the test suite using the given function.
 */
#define AddTestCase(suite, func) cu_add_test_case(suite, func, #func)

#define FailFast(tc, cond, expr1, expr2)                         \
    do                                                           \
    {                                                            \
        if (!(cond))                                             \
        {                                                        \
            cu_set_failed(tc, __FILE__, __LINE__, expr1, expr2); \
            return;                                              \
        }                                                        \
    } while (0)

#define AssertEqual(tc, first, second) \
    FailFast(tc, cu_assert_equal(tc, first, second, 0), #first, #second)

#define AssertNotEqual(tc, first, second) \
    FailFast(tc, cu_assert_equal(tc, first, second, 1), #first, #second)

#define AssertStrEqual(tc, first, second) \
    FailFast(tc, cu_assert_str_equal(tc, first, second, 0), #first, #second)

#define AssertStrNotEqual(tc, first, second) \
    FailFast(tc, cu_assert_str_equal(tc, first, second, 1), #first, #second)

#define AssertTrue(tc, expr) \
    FailFast(tc, cu_assert_true(tc, expr, 0), #expr, NULL)

#define AssertFalse(tc, expr) \
    FailFast(tc, cu_assert_true(tc, expr, 1), #expr, NULL)

#define AssertNull(tc, ptr) \
    FailFast(tc, cu_assert_null(tc, ptr, 0), #ptr, NULL)

#define AssertNotNull(tc, ptr) \
    FailFast(tc, cu_assert_null(tc, ptr, 1), #ptr, NULL)

/**
 * A single test case.
 */
typedef struct cu_test_case CuTestCase;

/**
 * A series of test cases.
 */
typedef struct cu_test_suite CuTestSuite;

/**
 * Stateful timer function pointer that measures time elapsed in seconds.
 *
 * If passed a non-zero value, then the timer shall be reset and 0.0 returned.
 * Otherwise, the time elapsed since the most recent reset shall be returned.
 */
typedef float (*timer_fn_t)(int);

extern CuTestSuite *cu_new_test_suite(void);

extern void cu_add_test_case(
    CuTestSuite *suite, void (*func)(CuTestCase *), char const *name);

/**
 * Run all tests in the test suite and return the number of failures.
 *
 * If the output stream is not null, then summary information is printed as each
 * test completes.
 *
 * If the timer function is not null, then the time elapsed shall be recorded.
 */
extern unsigned int cu_run_tests(
    CuTestSuite *suite, FILE *out, timer_fn_t timer);

/**
 * Print information about test results to the output stream.
 */
extern void cu_print_results(CuTestSuite const *suite, FILE *out);

extern int cu_assert_equal(CuTestCase *tc, int i1, int i2, int negate);

extern int cu_assert_str_equal(
    CuTestCase *tc, char const *s1, char const *s2, int negate);

extern int cu_assert_true(CuTestCase *tc, int b, int negate);

extern int cu_assert_null(CuTestCase *tc, void const *p, int negate);

extern void cu_set_failed(
    CuTestCase *tc, char const *file, int line,
    char const *expr1, char const *expr2);

#endif
