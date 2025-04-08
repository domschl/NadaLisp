// tests/test_runner.h
#ifndef TEST_RUNNER_H
#define TEST_RUNNER_H

#include "../src/NadaEval.h"
#include "../src/NadaParser.h"
#include "../src/NadaValue.h"

// Initialize testing environment
void init_test_env();

// Clean up testing environment
void cleanup_test_env();

// Run a single test with an expected result
int run_test(const char *name, const char *expr, const char *expected);

// Report test results
void report_results();

// Export the test count variables (without static)
extern int tests_run;
extern int tests_passed;

#endif  // TEST_RUNNER_H