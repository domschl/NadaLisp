// tests/test_list.c
#include "test_runner.h"
#include <stdio.h>

// Add required function declarations from test_runner.c
extern void init_test_env();
extern void cleanup_test_env();
extern int run_test(const char *name, const char *expr, const char *expected);
extern void report_results();

int test_car() {
    int success = 1;
    printf("DEBUG: Testing with expression: %s\n", "(car '(1 2 3))");
    NadaValue *parsed = nada_parse("(car '(1 2 3))");
    printf("DEBUG: Parsed as: ");
    nada_print(parsed);
    printf("\n");
    success &= run_test("simple_car", "(car '(1 2 3))", "1");
    printf("DEBUG: Testing with expression: %s\n", "(car '((1 2) 3 4))");
    parsed = nada_parse("(car '((1 2) 3 4))");
    printf("DEBUG: Parsed as: ");
    nada_print(parsed);
    printf("\n");
    success &= run_test("car_of_nested", "(car '((1 2) 3 4))", "(1 2)");
    return success;
}

int test_cdr() {
    int success = 1;
    printf("DEBUG: Testing with expression: %s\n", "(cdr '(1 2 3))");
    NadaValue *parsed = nada_parse("(cdr '(1 2 3))");
    printf("DEBUG: Parsed as: ");
    nada_print(parsed);
    printf("\n");
    success &= run_test("simple_cdr", "(cdr '(1 2 3))", "(2 3)");
    printf("DEBUG: Testing with expression: %s\n", "(cdr '(1))");
    parsed = nada_parse("(cdr '(1))");
    printf("DEBUG: Parsed as: ");
    nada_print(parsed);
    printf("\n");
    success &= run_test("cdr_at_end", "(cdr '(1))", "()");
    return success;
}

int test_cons() {
    int success = 1;
    printf("DEBUG: Testing with expression: %s\n", "(cons 1 '(2 3))");
    NadaValue *parsed = nada_parse("(cons 1 '(2 3))");
    printf("DEBUG: Parsed as: ");
    nada_print(parsed);
    printf("\n");
    success &= run_test("cons_to_list", "(cons 1 '(2 3))", "(1 2 3)");
    printf("DEBUG: Testing with expression: %s\n", "(cons 1 '())");
    parsed = nada_parse("(cons 1 '())");
    printf("DEBUG: Parsed as: ");
    nada_print(parsed);
    printf("\n");
    success &= run_test("cons_empty_list", "(cons 1 '())", "(1)");
    printf("DEBUG: Testing with expression: %s\n", "(cons 1 2)");
    parsed = nada_parse("(cons 1 2)");
    printf("DEBUG: Parsed as: ");
    nada_print(parsed);
    printf("\n");
    success &= run_test("cons_atom", "(cons 1 2)", "(1 . 2)");
    return success;
}

int test_list() {
    int success = 1;
    printf("DEBUG: Testing with expression: %s\n", "(list 1 2 3)");
    NadaValue *parsed = nada_parse("(list 1 2 3)");
    printf("DEBUG: Parsed as: ");
    nada_print(parsed);
    printf("\n");
    success &= run_test("create_list", "(list 1 2 3)", "(1 2 3)");
    printf("DEBUG: Testing with expression: %s\n", "(list)");
    parsed = nada_parse("(list)");
    printf("DEBUG: Parsed as: ");
    nada_print(parsed);
    printf("\n");
    success &= run_test("create_empty_list", "(list)", "()");
    return success;
}

int main() {
    init_test_env();

    int success = 1;
    success &= test_car();
    success &= test_cdr();
    success &= test_cons();
    success &= test_list();

    report_results();
    cleanup_test_env();

    return tests_passed == tests_run ? 0 : 1;
}