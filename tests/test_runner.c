// tests/test_runner.c
#include "../src/NadaEval.h"
#include "../src/NadaParser.h"
#include "../src/NadaValue.h"
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

// Helper to check if two values are equal (with improved dotted pair handling)
int values_equal(NadaValue *a, NadaValue *b) {
    if (a->type != b->type) {
        return 0;
    }

    switch (a->type) {
    case NADA_NUM:
        // Use the rational number equality function
        return nada_num_equal(a->data.number, b->data.number);
    case NADA_BOOL:
        return a->data.boolean == b->data.boolean;
    case NADA_STRING:
        return strcmp(a->data.string, b->data.string) == 0;
    case NADA_SYMBOL:
        return strcmp(a->data.symbol, b->data.symbol) == 0;
    case NADA_NIL:
        return 1;  // Both are nil
    case NADA_PAIR:
        // First check the car values recursively
        if (!values_equal(a->data.pair.car, b->data.pair.car)) {
            return 0;
        }

        // For dotted pairs, directly compare the cdr values
        return values_equal(a->data.pair.cdr, b->data.pair.cdr);
    default:
        return 0;
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