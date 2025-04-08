// tests/test_io.c
#include "test_runner.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Add required function declarations from test_runner.c
extern void init_test_env();
extern void cleanup_test_env();
extern int run_test(const char *name, const char *expr, const char *expected);
extern void report_results();

// Test file operations
int test_file_operations() {
    int success = 1;

    // Write to a test file
    success &= run_test("write_file",
                        "(write-file \"test_output.txt\" \"Hello, World!\")",
                        "#t");

    // Read the same file back and verify content
    success &= run_test("read_file",
                        "(read-file \"test_output.txt\")",
                        "\"Hello, World!\"");

    return success;
}

// Test string conversions
int test_string_conversions() {
    int success = 1;

    // Number to string
    success &= run_test("number_to_string",
                        "(number->string 42)",
                        "\"42\"");

    // String to number
    success &= run_test("string_to_number",
                        "(string->number \"42\")",
                        "42");

    // Invalid string to number
    success &= run_test("invalid_string_to_number",
                        "(string->number \"abc\")",
                        "#f");

    return success;
}

int main() {
    init_test_env();

    int success = 1;
    success &= test_file_operations();
    success &= test_string_conversions();

    report_results();
    cleanup_test_env();

    return tests_passed == tests_run ? 0 : 1;
}