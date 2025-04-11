#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>

#include "../src/NadaError.h"
#include "../src/NadaEval.h"
#include "../src/NadaParser.h"
#include "../src/NadaValue.h"
#include "../src/NadaConfig.h"

// Define test variables
static int tests_run = 0;
static int tests_passed = 0;

// Replace the old error tracking with the new system
static int had_evaluation_error = 0;

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

    // Reset test counters
    tests_run = 0;
    tests_passed = 0;
}

static void report_results() {
    printf("\n==== Test Summary ====\n");
    printf("Ran %d tests\n", tests_run);
    printf("Passed: %d\n", tests_passed);
    printf("Failed: %d\n", tests_run - tests_passed);
    printf("========================\n");
}

// Run a single test file
static int run_test_file(const char *filename) {
    printf("Running tests from %s\n", filename);
    // Load library files - similar to what we do in the REPL
    NadaEnv *env = nada_create_standard_env();  // Change this to use standard env

    nada_load_libraries(env);
    // Reset error flag
    reset_error_flag();

    // Store current test count
    int tests_before = tests_run;

    // Load and evaluate the file with additional error handling
    NadaValue *result = NULL;

    // Set up a signal handler or use setjmp/longjmp for crash protection if needed

    nada_set_silent_symbol_lookup(1);  // Suppress symbol lookup errors
    // Load and evaluate with careful error handling
    result = nada_load_file(filename, env);
    nada_set_silent_symbol_lookup(0);  // Suppress symbol lookup errors

    // Check for errors
    int success = !had_evaluation_error && result != NULL;

    // Calculate tests run in this file
    int file_tests = tests_run - tests_before;
    printf("Ran %d tests from %s\n", file_tests, filename);

    // Clean up
    if (result) nada_free(result);
    nada_cleanup_env(env);  // Use the cleanup function
    env = NULL;             // Reset the environment
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

    // Open the directory
    dir = opendir(dir_path);
    if (!dir) {
        fprintf(stderr, "Error: Could not open directory: %s\n", dir_path);
        return 0;
    }

    // Process each .scm file
    while ((entry = readdir(dir)) != NULL) {
        // Skip non-lisp files
        if (!strstr(entry->d_name, ".scm")) continue;

        // Build full path
        char full_path[1024];
        snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, entry->d_name);

        // Validate test file
        validate_test_file(full_path);

        // Run tests in this file
        if (!run_test_file(full_path)) {
            all_passed = 0;
        }
    }

    closedir(dir);

    // Show combined results
    report_results();

    // Report test coverage
    report_test_coverage(dir_path);

    return all_passed;
}

// Main function if running directly
int main(int argc, char *argv[]) {
    printf("=== NadaLisp Test Runner (Debug Version) ===\n");

    // Register an atexit handler to ensure cleanup happens even on crashes
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

    return result ? 0 : 1;
}