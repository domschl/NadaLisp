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
    success &= run_test("integer_division", "(/ 10 3)", "3");  // Integer division
    return success;
}

int main() {
    init_test_env();

    int success = 1;
    success &= test_addition();
    success &= test_subtraction();
    success &= test_multiplication();
    success &= test_division();

    report_results();
    cleanup_test_env();

    return tests_passed == tests_run ? 0 : 1;
}