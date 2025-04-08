#include "../src/NadaEval.h"
#include "../src/NadaParser.h"
#include "../src/NadaValue.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

// Import test functions from test_runner.c
extern int tests_run;
extern int tests_passed;
extern void init_test_env();
extern void cleanup_test_env();
extern void report_results();
extern int values_equal(NadaValue *a, NadaValue *b);
extern NadaEnv *test_env;

// Setup test environment with testing functions
static void setup_test_env(NadaEnv *env) {
    // Register assert-equal function
    NadaValue *expr = nada_parse(
        "(define (assert-equal actual expected) "
        "  (if (equal? actual expected) "
        "      #t "
        "      (begin "
        "        (display \"  Expected: \") "
        "        (display expected) "
        "        (display \", Got: \") "
        "        (display actual) "
        "        (display \"\\n\") "
        "        #f)))");
    NadaValue *result = nada_eval(expr, env);
    nada_free(expr);
    nada_free(result);

    // Register define-test function
    expr = nada_parse(
        "(define (define-test name body) "
        "  (display \"Test: \") "
        "  (display name) "
        "  (display \"... \") "
        "  (if body "
        "      (display \"PASSED\\n\") "
        "      (display \"FAILED\\n\")) "
        "  body)");
    result = nada_eval(expr, env);
    nada_free(expr);
    nada_free(result);
}

// Run a single test file
static int run_test_file(const char *filename, NadaEnv *env) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Error: Could not open test file: %s\n", filename);
        return 0;
    }

    printf("Running tests from %s\n", filename);

    char buffer[10240] = {0};
    char line[1024];
    int paren_balance = 0;
    int in_string = 0;
    int file_tests_passed = 0;
    int file_tests_run = 0;

    while (fgets(line, sizeof(line), file)) {
        // Skip comments and empty lines
        if (line[0] == ';' || line[0] == '\n') continue;

        // Update buffer with this line
        strcat(buffer, line);

        // Count parentheses and check for strings
        for (char *p = line; *p; p++) {
            if (*p == '"') in_string = !in_string;
            if (!in_string) {
                if (*p == '(')
                    paren_balance++;
                else if (*p == ')')
                    paren_balance--;
            }
        }

        // If balanced, process the expression
        if (paren_balance == 0 && strlen(buffer) > 0) {
            // Parse as NadaValue
            NadaValue *expr = nada_parse(buffer);

            // Check if this is a test definition
            if (expr->type == NADA_PAIR &&
                nada_car(expr)->type == NADA_SYMBOL &&
                strcmp(nada_car(expr)->data.symbol, "define-test") == 0) {

                tests_run++;
                file_tests_run++;

                // Evaluate the test
                NadaValue *result = nada_eval(expr, env);

                // Check result
                if (result->type == NADA_BOOL && result->data.boolean) {
                    tests_passed++;
                    file_tests_passed++;
                }

                nada_free(result);
            } else {
                // Just evaluate non-test expressions
                NadaValue *result = nada_eval(expr, env);
                nada_free(result);
            }

            nada_free(expr);
            buffer[0] = '\0';  // Reset buffer for next expression
        }
    }

    fclose(file);
    printf("File results: %d/%d tests passed\n\n", file_tests_passed, file_tests_run);
    return file_tests_passed == file_tests_run;
}

// Run all test files in a directory
int run_lisp_tests(const char *dir_path) {
    DIR *dir;
    struct dirent *entry;
    int all_passed = 1;

    // Initialize test environment
    init_test_env();

    // Add testing functions to environment
    setup_test_env(test_env);

    // Open the directory
    dir = opendir(dir_path);
    if (!dir) {
        fprintf(stderr, "Error: Could not open directory: %s\n", dir_path);
        return 0;
    }

    // Process each .lisp file
    while ((entry = readdir(dir)) != NULL) {
        // Skip non-lisp files
        if (!strstr(entry->d_name, ".lisp")) continue;

        // Build full path
        char full_path[1024];
        snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, entry->d_name);

        // Run tests in this file
        if (!run_test_file(full_path, test_env)) {
            all_passed = 0;
        }
    }

    closedir(dir);

    // Show combined results
    report_results();

    // Cleanup
    cleanup_test_env();

    return all_passed;
}

// Main function if running directly
int main(int argc, char *argv[]) {
    const char *test_dir = (argc > 1) ? argv[1] : "tests/lisp_tests";
    return run_lisp_tests(test_dir) ? 0 : 1;
}