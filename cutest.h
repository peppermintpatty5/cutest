#ifndef CUTEST_H_
#define CUTEST_H_

#include <stdio.h>

#define AddTestCase(suite, func) \
    cu_add_test(suite, cu_new_test_case(func, #func))

#define AssertTrue(tc, expr) \
    CuAssert(tc, CU_ASSERT_TRUE, expr, 0, i)

#define AssertEqual(tc, first, second) \
    CuAssert(tc, CU_ASSERT_EQUAL, first, second, i)

#define CuAssert(tc, type, expr1, expr2, X)                               \
    do                                                                    \
    {                                                                     \
        CuAssertArg arg1, arg2;                                           \
        arg1.expr = #expr1;                                               \
        arg2.expr = #expr2;                                               \
        arg1.value.X = expr1;                                             \
        arg2.value.X = expr2;                                             \
        if (!cu_do_assertion(tc, type, &arg1, &arg2, __FILE__, __LINE__)) \
            return;                                                       \
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
typedef enum
{
    CU_ASSERT_EQUAL,
    CU_ASSERT_TRUE
} CuAssertType;

/**
 * Represents an argument passed to an assertion function.
 */
typedef struct
{
    /* The argument's original string representation */
    char const *expr;
    /* The argument's actual value */
    union cu_value
    {
        void const *p;
        int i;
    } value;
} CuAssertArg;

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
 * Perform the assertion check.
 *
 * If the assertion fails, then the test case is updated as such and 0 is
 * returned.
 */
extern int cu_do_assertion(
    CuTestCase *tc, CuAssertType type,
    CuAssertArg *arg1, CuAssertArg *arg2,
    char const *file, int line);

/**
 * Run all tests in the test suite.
 */
extern void cu_run_tests(CuTestSuite *suite);

/**
 * Print information about test results to the output stream.
 */
extern void cu_print_results(CuTestSuite *suite, FILE *out);

#endif
