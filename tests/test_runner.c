// tests/test_runner.c
#include "../src/NadaEval.h"
#include "../src/NadaParser.h"
#include "../src/NadaValue.h"
#include "../src/NadaBuiltinCompare.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Globals for tracking test progress
int tests_run = 0;
int tests_passed = 0;

// Test environment
NadaEnv *test_env = NULL;

// Initialize testing environment
void init_test_env() {
    if (test_env == NULL) {
        test_env = nada_create_standard_env();
    }
}

// Clean up testing environment
void cleanup_test_env() {
    if (test_env != NULL) {
        nada_env_free(test_env);
        test_env = NULL;
    }
}


// Run a single test with an expected result
int run_test(const char *name, const char *expr, const char *expected) {
    printf("Test: %s... ", name);
    tests_run++;

    NadaValue *parsed_expr = nada_parse(expr);
    NadaValue *result = nada_eval(parsed_expr, test_env);
    nada_free(parsed_expr);

    NadaValue *parsed_expected = nada_parse(expected);
    int success = values_equal(result, parsed_expected);

    if (success) {
        printf("PASSED\n");
        tests_passed++;
    } else {
        printf("FAILED\n");
        printf("  Expected: ");
        nada_print(parsed_expected);
        printf("\n  Got: ");
        nada_print(result);
        printf("\n");
    }

    nada_free(result);
    nada_free(parsed_expected);

    return success;
}

// Report test results
void report_results() {
    printf("\n==== Test Summary ====\n");
    printf("Ran %d tests\n", tests_run);
    printf("Passed: %d\n", tests_passed);
    printf("Failed: %d\n", tests_run - tests_passed);
    printf("========================\n");
}