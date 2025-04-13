#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>

#include "NadaError.h"
#include "NadaEval.h"
#include "NadaParser.h"
#include "NadaValue.h"
#include "NadaConfig.h"

static NadaEnv *global_env = NULL;
static int had_evaluation_error = 0;
static NadaValue *test_results_var;

// Error handler callback
static void test_error_handler(NadaErrorType type, const char *message, void *user_data) {
    // Do NOT error out if we are in silent mode, used for testing handling of undefined symbols
    if (!nada_is_global_silent_symbol_lookup()) {
        // Print the error message
        fprintf(stderr, "Test-Handler-Error: %s\n", message);
        // Set the flag to indicate an error occurred
        had_evaluation_error = 1;
    } else {
        printf("Suppressing lookup-error: %s\n", message);
    }
}

// Reset the error flag
static void reset_error_flag() {
    had_evaluation_error = 0;
}

// Initialize test environment
static void init_test_env() {
    // Use the function from NadaEval.h
    // Register our error handler
    nada_set_error_handler(test_error_handler, NULL);

    // Reset error tracking
    reset_error_flag();
}

static void report_results() {
}

// Run a single test file
static int run_test_file(const char *filename) {
    printf("Running tests from %s\n", filename);
    // Load library files - similar to what we do in the REPL
    global_env = nada_create_standard_env();  // Change this to use standard env

    printf("Loading libraries\n");
    nada_load_libraries(global_env);

    printf("Setting results boolean\n");
    // Add a global variable to track test results
    NadaValue *test_results_var = nada_create_bool(1);  // Start with true (all passed)
    nada_env_set(global_env, "tests-all-passed", test_results_var);

    printf("Setting test count\n");
    // Add a counter for number of tests
    NadaValue *tests_run_var = nada_create_num_from_int(0);
    nada_env_set(global_env, "tests-run-count", tests_run_var);

    // Reset error flag
    reset_error_flag();

    // Store current test count
    int tests_before = 0;  // tests_run;

    // Load and evaluate the file with additional error handling
    NadaValue *result = NULL;

    printf("Loading file: %s\n", filename);
    // Load and evaluate with careful error handling
    result = nada_load_file(filename, global_env);

    printf("Getting test results\n");
    // Check for errors or test failures
    NadaValue *final_status = nada_env_get(global_env, "tests-all-passed", 1);  // Set silent flag
    int all_tests_passed = final_status && final_status->type == NADA_BOOL &&
                           final_status->data.boolean == 1;

    NadaValue *passed_count = nada_env_get(global_env, "tests-passed-count", 1);
    NadaValue *failed_count = nada_env_get(global_env, "tests-failed-count", 1);

    int tests_passed = 0;
    int tests_failed = 0;

    if (passed_count && passed_count->type == NADA_NUM) {
        tests_passed = nada_num_to_int(passed_count->data.number);
    }

    if (failed_count && failed_count->type == NADA_NUM) {
        tests_failed = nada_num_to_int(failed_count->data.number);
    }

    // Get tests run count - use silent lookup
    NadaValue *test_count = nada_env_get(global_env, "tests-run-count", 1);

    int file_tests = 0;
    if (test_count && test_count->type == NADA_NUM) {
        // This line is causing the issue - incorrect field access
        file_tests = nada_num_to_int(test_count->data.number);
    }

    int success = !had_evaluation_error && result != NULL && all_tests_passed;

    printf("Test file %s, test-count: %d\n", filename, file_tests);

    // Update test counters
    int tests_run;
    tests_run += file_tests;
    if (all_tests_passed) {
        tests_passed += file_tests;
    }

    // Free values
    if (final_status) nada_free(final_status);
    if (test_count) nada_free(test_count);

    printf("Ran %d tests from %s\n", file_tests, filename);
    printf("\n==== Test Summary ====\n");
    printf("Ran %d tests\n", tests_run);
    printf("Passed: %d\n", tests_passed);
    printf("Failed: %d\n", tests_run - tests_passed);
    printf("========================\n");

    // Clean up
    if (result) nada_free(result);
    nada_cleanup_env(global_env);
    global_env = NULL;  // Reset the environment
    return success;
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

    // Process each .scm file to count
    while ((entry = readdir(dir)) != NULL) {
        // Skip non-lisp files
        if (!strstr(entry->d_name, ".scm")) continue;

        total_files++;
        // Check if file was executed (you'd need to track this)
        // covered_files++;
    }

    rewinddir(dir);

    closedir(dir);
    // Print details of files
    printf("Test files found: %d\n", total_files);
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

// Main function if running directly
int main(int argc, char *argv[]) {
    printf("=== NadaLisp Test Runner ===\n");

    // If a specific file is provided, just run that file
    if (argc > 1 && strstr(argv[1], ".scm")) {
        // Initialize test environment
        init_test_env();

        validate_test_file(argv[1]);

        // Run the specific test file
        int result = run_test_file(argv[1]);

        return result ? 0 : 1;
    } else {
        printf("Usage: %s [test-file.scm]\n", argv[0]);
        return 1;
    }
}