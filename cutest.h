#ifndef CUTEST_H_
#define CUTEST_H_

#include <stdio.h>

#define AddTestCase(suite, func) \
    cu_add_test(suite, cu_new_test_case(func, #func))

#define AssertTrue(tc, expr) \
    CuAssert((tc), CU_ASSERT_TRUE, #expr, NULL, (expr))

#define AssertEqual(tc, first, second) \
    CuAssert((tc), CU_ASSERT_EQUAL, #first, #second, (first) == (second))

#define CuAssert(tc, type, arg1, arg2, expr)                                 \
    do                                                                       \
    {                                                                        \
        if (!(expr))                                                         \
        {                                                                    \
            cu_set_failed((tc), (type), __FILE__, __LINE__, (arg1), (arg2)); \
            return;                                                          \
        }                                                                    \
    } while (0)

/**
 * A single test case.
 */
typedef struct cu_test_case CuTestCase;

/**
 * A series of test cases.
 */
typedef struct cu_test_suite CuTestSuite;

/**
 * All possible assertion types.
 */
enum cu_assert_type
{
    CU_ASSERT_EQUAL,
    CU_ASSERT_TRUE
};

/**
 * Create a new test suite.
 */
extern CuTestSuite *cu_new_test_suite(void);

/**
 * Create a new test case using the given function.
 */
extern CuTestCase *cu_new_test_case(void (*func)(CuTestCase *), char const *name);

/**
 * Add test case to test suite.
 */
extern void cu_add_test(CuTestSuite *suite, CuTestCase *tc);

/**
 * Denote the test case as failing and update the corresponding failure
 * information.
 */
extern void cu_set_failed(
    CuTestCase *tc, enum cu_assert_type type,
    char const *file, int line,
    char const *arg1, char const *arg2);

/**
 * Run all tests in the test suite.
 */
extern void cu_run_tests(CuTestSuite *suite);

/**
 * Print information about test results to the output stream.
 */
extern void cu_print_results(CuTestSuite *suite, FILE *out);

#endif
