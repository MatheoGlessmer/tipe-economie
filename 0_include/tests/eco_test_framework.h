#ifndef ECO_TEST_FRAMEWORK_H
#define ECO_TEST_FRAMEWORK_H

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#define ECO_TEST_COLOR_GREEN   "\033[32m"
#define ECO_TEST_COLOR_RED     "\033[31m"
#define ECO_TEST_COLOR_YELLOW  "\033[33m"
#define ECO_TEST_COLOR_MAGENTA "\033[35m"
#define ECO_TEST_COLOR_CYAN    "\033[36m"
#define ECO_TEST_COLOR_BOLD    "\033[1m"
#define ECO_TEST_COLOR_RESET   "\033[0m"

typedef enum EcoTestStatus {
    ECO_TEST_STATUS_PASS = 0,
    ECO_TEST_STATUS_FAIL,
    ECO_TEST_STATUS_TODO,
    ECO_TEST_STATUS_CRASH
} EcoTestStatus;

typedef struct EcoTestResult {
    EcoTestStatus status;
    const char *message;
    const char *file;
    int line;
} EcoTestResult;

typedef EcoTestResult (*EcoTestFunction)(void);

typedef struct EcoTestCase {
    const char *suite_name;
    const char *test_name;
    EcoTestFunction function;
} EcoTestCase;

typedef struct EcoTestSummary {
    size_t passed;
    size_t failed;
    size_t todo;
    size_t crashed;
    size_t total;
} EcoTestSummary;

EcoTestResult eco_test_make_result(EcoTestStatus status,
                                   const char *message,
                                   const char *file,
                                   int line);

const char *eco_test_status_name(EcoTestStatus status);
const char *eco_test_status_color(EcoTestStatus status);

int eco_test_run_all(const EcoTestCase *tests,
                     size_t test_count,
                     bool use_color,
                     EcoTestSummary *summary);

#define ECO_TEST_PASS(message) \
    return eco_test_make_result(ECO_TEST_STATUS_PASS, (message), __FILE__, __LINE__)

#define ECO_TEST_FAIL(message) \
    return eco_test_make_result(ECO_TEST_STATUS_FAIL, (message), __FILE__, __LINE__)

#define ECO_TEST_TODO(message) \
    return eco_test_make_result(ECO_TEST_STATUS_TODO, (message), __FILE__, __LINE__)

#define ECO_TEST_ASSERT(condition, message) \
    do { \
        if (!(condition)) { \
            return eco_test_make_result(ECO_TEST_STATUS_FAIL, (message), __FILE__, __LINE__); \
        } \
    } while (0)

#define ECO_TEST_ASSERT_NOT_NULL(pointer, message) \
    ECO_TEST_ASSERT((pointer) != NULL, (message))

#define ECO_TEST_ASSERT_NULL(pointer, message) \
    ECO_TEST_ASSERT((pointer) == NULL, (message))

#define ECO_TEST_ASSERT_EQ_INT(expected, actual, message) \
    ECO_TEST_ASSERT((expected) == (actual), (message))

#define ECO_TEST_ASSERT_EQ_REAL(expected, actual, message) \
    ECO_TEST_ASSERT((expected) == (actual), (message))

#define ECO_TEST_ASSERT_APPROX_REAL(expected, actual, tolerance, message) \
    do { \
        long long eco_test_diff__ = (long long)((expected) - (actual)); \
        if (eco_test_diff__ < 0) { \
            eco_test_diff__ = -eco_test_diff__; \
        } \
        ECO_TEST_ASSERT(eco_test_diff__ <= (long long)(tolerance), (message)); \
    } while (0)

#endif /* ECO_TEST_FRAMEWORK_H */
