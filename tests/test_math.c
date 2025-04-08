// tests/test_math.c
#include "test_runner.h"
#include <stdio.h>

// Add required function declarations from test_runner.c
extern void init_test_env();
extern void cleanup_test_env();
extern int run_test(const char *name, const char *expr, const char *expected);
extern void report_results();

int test_addition() {
    int success = 1;
    success &= run_test("simple_addition", "(+ 1 2)", "3");
    success &= run_test("multi_addition", "(+ 1 2 3 4)", "10");
    success &= run_test("nested_addition", "(+ (+ 1 2) (+ 3 4))", "10");
    return success;
}

int test_subtraction() {
    int success = 1;
    success &= run_test("simple_subtraction", "(- 5 2)", "3");
    success &= run_test("negative_result", "(- 2 5)", "-3");
    success &= run_test("multi_subtraction", "(- 10 2 3)", "5");
    return success;
}

int test_multiplication() {
    int success = 1;
    success &= run_test("simple_multiplication", "(* 3 4)", "12");
    success &= run_test("multi_multiplication", "(* 2 3 4)", "24");
    return success;
}

int test_division() {
    int success = 1;
    success &= run_test("simple_division", "(/ 10 2)", "5");
    success &= run_test("rational_division", "(/ 10 3)", "10/3");  // Now returns exact rational

    // Add tests for the new rational number capabilities
    success &= run_test("simple_fraction", "(/ 1 2)", "1/2");
    success &= run_test("reduced_fraction", "(/ 4 8)", "1/2");  // Should be automatically reduced
    success &= run_test("negative_fraction", "(/ -1 4)", "-1/4");
    success &= run_test("negative_denominator", "(/ 1 -4)", "-1/4");  // Result should have standard sign format

    return success;
}

int test_rational_arithmetic() {
    int success = 1;

    // Adding fractions
    success &= run_test("add_fractions", "(+ 1/3 1/6)", "1/2");
    success &= run_test("add_mixed", "(+ 2/3 1)", "5/3");

    // Subtracting fractions
    success &= run_test("subtract_fractions", "(- 7/8 1/4)", "5/8");

    // Multiplying fractions
    success &= run_test("multiply_fractions", "(* 2/3 3/4)", "1/2");

    // More complex expressions
    success &= run_test("complex_fraction_expr", "(/ (+ 1 2) (- 6 3))", "1");
    success &= run_test("mixed_arithmetic", "(+ (* 1/2 1/3) (/ 1 6))", "1/3");

    return success;
}

int main() {
    init_test_env();

    int success = 1;
    success &= test_addition();
    success &= test_subtraction();
    success &= test_multiplication();
    success &= test_division();
    success &= test_rational_arithmetic();

    report_results();
    cleanup_test_env();

    return tests_passed == tests_run ? 0 : 1;
}