// tests/test_string.c
#include "test_runner.h"
#include <stdio.h>

// Add required function declarations from test_runner.c
extern void init_test_env();
extern void cleanup_test_env();
extern int run_test(const char *name, const char *expr, const char *expected);
extern void report_results();

int test_string_length() {
    int success = 1;
    success &= run_test("empty_string_length", "(string-length \"\")", "0");
    success &= run_test("simple_string_length", "(string-length \"hello\")", "5");
    success &= run_test("utf8_string_length", "(string-length \"Tö\")", "2");
    return success;
}

int test_substring() {
    int success = 1;
    success &= run_test("simple_substring", "(substring \"hello\" 1 3)", "\"ell\"");
    success &= run_test("substring_to_end", "(substring \"hello\" 1)", "\"ello\"");
    return success;
}

int test_string_split() {
    int success = 1;
    success &= run_test("split_chars", "(string-split \"abc\")", "(\"a\" \"b\" \"c\")");
    success &= run_test("split_by_delimiter",
                        "(string-split \"aa12bb12cc\" \"12\")",
                        "(\"aa\" \"bb\" \"cc\")");
    return success;
}

int test_string_join() {
    int success = 1;
    success &= run_test("join_simple", "(string-join '(\"hello\" \"world\"))", "\"helloworld\"");
    success &= run_test("join_with_delimiter",
                        "(string-join '(\"hello\" \"world\") \" \")",
                        "\"hello world\"");
    return success;
}

int main() {
    init_test_env();

    int success = 1;
    success &= test_string_length();
    success &= test_substring();
    success &= test_string_split();
    success &= test_string_join();

    report_results();
    cleanup_test_env();

    return tests_passed == tests_run ? 0 : 1;
}