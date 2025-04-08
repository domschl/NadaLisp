#include "../src/NadaEval.h"  // Include this for environment functions
#include "../src/NadaParser.h"
#include "../src/NadaValue.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include "../src/NadaError.h"

// Define test variables locally instead of importing them
static NadaEnv *test_env;
static int tests_run = 0;
static int tests_passed = 0;

// Replace the old error tracking with the new system
static int had_evaluation_error = 0;

// Error handler callback
static void test_error_handler(NadaErrorType type, const char *message, void *user_data) {
    // Print the error message
    fprintf(stderr, "Error: %s\n", message);

    // Set the flag to indicate an error occurred
    had_evaluation_error = 1;
}

// Reset the error flag
static void reset_error_flag() {
    had_evaluation_error = 0;
}

// Function to get type name as string (missing in your headers)
static const char *nada_type_name(int type) {
    switch (type) {
    case NADA_NIL:
        return "NIL";
    case NADA_BOOL:
        return "BOOLEAN";
    case NADA_NUM:
        return "NUMBER";
    case NADA_SYMBOL:
        return "SYMBOL";
    case NADA_STRING:
        return "STRING";
    case NADA_PAIR:
        return "PAIR";
    case NADA_FUNC:
        return "FUNCTION";
    default:
        return "UNKNOWN";
    }
}

// Initialize test environment
static void init_test_env() {
    // Use the function from NadaEval.h
    test_env = nada_create_standard_env();  // Change this to use standard env

    // Register our error handler
    nada_set_error_handler(test_error_handler, NULL);

    // Reset error tracking
    reset_error_flag();

    // Reset test counters
    tests_run = 0;
    tests_passed = 0;
}

// Cleanup the test environment
static void cleanup_test_env() {
    nada_env_free(test_env);
}

static void report_results() {
    printf("\n==== Test Summary ====\n");
    printf("Ran %d tests\n", tests_run);
    printf("Passed: %d\n", tests_passed);
    printf("Failed: %d\n", tests_run - tests_passed);
    printf("========================\n");
}

// A simplified version of values_equal since we're not importing it
static int values_equal(NadaValue *a, NadaValue *b) {
    if (a->type != b->type) return 0;

    switch (a->type) {
    case NADA_NIL:
        return 1;
    case NADA_BOOL:
        return a->data.boolean == b->data.boolean;
    case NADA_NUM:
        // This assumes you have a num_equal function
        return nada_num_equal(a->data.number, b->data.number);
    case NADA_SYMBOL:
        return strcmp(a->data.symbol, b->data.symbol) == 0;
    case NADA_STRING:
        return strcmp(a->data.string, b->data.string) == 0;
    case NADA_PAIR:
        // Recursively check car and cdr
        return values_equal(a->data.pair.car, b->data.pair.car) &&
               values_equal(a->data.pair.cdr, b->data.pair.cdr);
    case NADA_FUNC:
        // Functions are equal if they're the same object (simplistic)
        return a == b;
    default:
        return 0;
    }
}

// Setup test environment with testing functions that work with your implementation
static void setup_test_env(NadaEnv *env) {
    // Register assert-equal with better error reporting
    NadaValue *expr = nada_parse(
        "(define assert-equal "
        "  (lambda (actual expected) "
        "    (if (equal? actual expected) "
        "        #t "
        "        (lambda () "
        "          (display \"  ASSERTION FAILED\\n\") "
        "          (display \"  Expected: \") "
        "          (write expected) "
        "          (display \"\\n  Got:      \") "
        "          (write actual) "
        "          (display \"\\n\") "
        "          #f))))");
    NadaValue *result = nada_eval(expr, env);
    nada_free(expr);
    nada_free(result);

    // Register a more robust define-test function
    expr = nada_parse(
        "(define define-test "
        "  (lambda (name body) "
        "    (display \"Test: \") "
        "    (display name) "
        "    (let ((result body)) "
        "      (display \"... \") "
        "      (if (equal? result #t) "  // Strictly check for #t
        "          (display \"PASSED\\n\") "
        "          (display \"FAILED\\n\")) "
        "      result)))");

    // If 'let' is not implemented, use a simpler version
    if (nada_parse("(let ((x 1)) x)") == NULL) {
        expr = nada_parse(
            "(define define-test "
            "  (lambda (name body) "
            "    (display \"Test: \") "
            "    (display name) "
            "    (display \"... \") "
            "    (if (equal? body #t) "
            "        (display \"PASSED\\n\") "
            "        (display \"FAILED\\n\")) "
            "    body))");
    }

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
    int line_num = 0;

    while (fgets(line, sizeof(line), file)) {
        line_num++;

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

            // Skip if parsing failed
            if (!expr) {
                fprintf(stderr, "  ERROR: Failed to parse expression at line %d in %s\n",
                        line_num, filename);
                buffer[0] = '\0';
                continue;
            }

            // Check if this is a test definition
            if (expr->type == NADA_PAIR &&
                nada_car(expr)->type == NADA_SYMBOL &&
                strcmp(nada_car(expr)->data.symbol, "define-test") == 0) {

                // Get test name for better reporting
                NadaValue *test_name = NULL;
                if (!nada_is_nil(nada_cdr(expr)) &&
                    nada_car(nada_cdr(expr))->type == NADA_STRING) {
                    test_name = nada_car(nada_cdr(expr));
                    printf("  Running test: %s (%s:%d)... ",
                           test_name->data.string, filename, line_num);
                } else {
                    printf("  Running unnamed test (%s:%d)... ", filename, line_num);
                }

                tests_run++;
                file_tests_run++;

                // Set current test info for better error reporting
                nada_env_set(env, "current-test-file", nada_create_string(filename));
                nada_env_set(env, "current-test-line", nada_create_num_from_int(line_num));

                // Reset error flag before evaluation
                reset_error_flag();

                // Evaluate the test
                NadaValue *result = nada_eval(expr, env);

                // Determine test success by checking both result and error flag
                int success = (result->type == NADA_BOOL && result->data.boolean && !had_evaluation_error);

                // Update the conditional for checking success
                if (success) {
                    tests_passed++;
                    file_tests_passed++;
                    printf("PASSED\n");
                } else {
                    // Detailed failure debug info
                    printf("FAILED\n");
                    printf("  Error flag: %s\n", had_evaluation_error ? "true (errors occurred)" : "false (no errors)");
                    printf("  Result type: %s\n", nada_type_name(result->type));
                    if (result->type == NADA_BOOL) {
                        printf("  Result value: %s\n", result->data.boolean ? "true" : "false");
                    }
                }

                nada_free(result);
            } else {
                // Just evaluate non-test expressions
                reset_error_flag();  // Reset before each evaluation
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

// Report test coverage details
static void report_test_coverage(const char *dir_path) {
    DIR *dir;
    struct dirent *entry;
    int total_files = 0;
    int covered_files = 0;

    // Open the directory
    dir = opendir(dir_path);
    if (!dir) {
        fprintf(stderr, "Error: Could not open directory: %s\n", dir_path);
        return;
    }

    printf("\n===== TEST COVERAGE REPORT =====\n");

    // Process each .lisp file to count
    while ((entry = readdir(dir)) != NULL) {
        // Skip non-lisp files
        if (!strstr(entry->d_name, ".lisp")) continue;

        total_files++;
        // Check if file was executed (you'd need to track this)
        // covered_files++;
    }

    rewinddir(dir);

    // Print details of files
    printf("Test files found: %d\n", total_files);
    // printf("Test files executed: %d (%.1f%%)\n", covered_files,
    //       (covered_files * 100.0) / total_files);

    // Add this call at the end of run_lisp_tests
    // before returning all_passed.
    closedir(dir);
}

// Validate test file syntax
static void validate_test_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) return;

    char line[1024];
    int line_num = 0;
    int define_test_count = 0;
    int assert_count = 0;

    printf("Validating %s:\n", filename);

    while (fgets(line, sizeof(line), file)) {
        line_num++;

        // Check for test definitions
        if (strstr(line, "define-test")) {
            define_test_count++;
        }

        // Check for assertions
        if (strstr(line, "assert-equal")) {
            assert_count++;
        }
    }

    printf("  Found %d tests with %d assertions\n",
           define_test_count, assert_count);

    fclose(file);
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

        // Validate test file
        validate_test_file(full_path);

        // Run tests in this file
        if (!run_test_file(full_path, test_env)) {
            all_passed = 0;
        }
    }

    closedir(dir);

    // Show combined results
    report_results();

    // Report test coverage
    report_test_coverage(dir_path);

    // Cleanup
    cleanup_test_env();

    return all_passed;
}

// Main function if running directly
int main(int argc, char *argv[]) {
    printf("=== NadaLisp Test Runner (Debug Version) ===\n");

    // Default test directory paths to try
    const char *test_paths[] = {
        // User-provided path
        argc > 1 ? argv[1] : NULL,
        // Common relative paths
        "lisp_tests",
        "tests/lisp_tests",
        "../tests/lisp_tests",
        "../../tests/lisp_tests",
        NULL};

    // Try each path until we find one that exists
    const char *test_dir = NULL;
    for (int i = 0; test_paths[i] != NULL; i++) {
        if (test_paths[i] == NULL) continue;

        DIR *dir = opendir(test_paths[i]);
        if (dir) {
            test_dir = test_paths[i];
            closedir(dir);
            break;
        }
    }

    if (test_dir == NULL) {
        fprintf(stderr, "Error: Could not find lisp_tests directory. Please specify path.\n");
        return 1;
    }

    printf("Using test directory: %s\n", test_dir);

    // Check files in directory
    DIR *debug_dir = opendir(test_dir);
    if (debug_dir) {
        struct dirent *entry;
        printf("Found files in test directory:\n");
        while ((entry = readdir(debug_dir)) != NULL) {
            printf("  %s\n", entry->d_name);
        }
        closedir(debug_dir);
    }

    // Optionally validate a specific test file if given
    if (argc > 2) {
        validate_test_file(argv[2]);
    }

    int result = run_lisp_tests(test_dir);

    // Generate coverage report
    report_test_coverage(test_dir);

    return result ? 0 : 1;
}