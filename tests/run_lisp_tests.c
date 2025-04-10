#include "../src/NadaEval.h"
#include "../src/NadaParser.h"
#include "../src/NadaValue.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include "../src/NadaError.h"

// Define test variables
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

// Add a library loading function similar to the one in NadaLisp.c
static void load_test_libraries() {
    // Try multiple potential library locations with more options
    const char *lib_dirs[] = {
        "src/nadalib",                // From project root
        "../src/nadalib",             // From build directory
        "../../src/nadalib",          // From build/tests
        "../../../src/nadalib",       // Deeper nested builds
        "./nadalib",                  // Local directory
        "/usr/local/share/nada/lib",  // System-wide
        NULL                          // End marker
    };

    DIR *dir = NULL;
    char cwd_buffer[1024];
    int found_index = -1;  // Store the index of the found directory

    // Get current working directory for better diagnostics
    if (getcwd(cwd_buffer, sizeof(cwd_buffer)) == NULL) {
        strcpy(cwd_buffer, "(unknown)");
    }

    printf("Searching for libraries from test working directory: %s\n", cwd_buffer);

    // Try each potential location
    for (int i = 0; lib_dirs[i] != NULL; i++) {
        dir = opendir(lib_dirs[i]);
        if (dir) {
            printf("Found library directory: %s\n", lib_dirs[i]);
            found_index = i;  // Store the index
            break;
        }
    }

    if (found_index < 0) {
        printf("Note: No library directory found. Tests may fail if they depend on libraries.\n");
        return;
    }

    // Use the actual found directory path for loading
    const char *found_dir = lib_dirs[found_index];
    printf("Loading libraries for tests from %s...\n", found_dir);

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        // Skip directories and non-.scm files
        if (entry->d_type == DT_DIR) continue;

        const char *filename = entry->d_name;
        size_t len = strlen(filename);

        // Check for .scm extension
        if (len > 4 && strcmp(filename + len - 4, ".scm") == 0) {
            // Use the correct found directory path
            char full_path[1024];
            snprintf(full_path, sizeof(full_path), "%s/%s", found_dir, filename);

            printf("  Loading %s\n", filename);
            NadaValue *result = nada_load_file(full_path, test_env);
            nada_free(result);

            // Reset error flag after each library load
            reset_error_flag();
        }
    }

    closedir(dir);
    printf("Libraries loaded for tests.\n");
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

    // Load library files - similar to what we do in the REPL
    load_test_libraries();
}

// Recursive function to break circular references in environments
static void break_env_cycles(NadaEnv *env, int depth) {
    if (!env || depth > 100) return;  // Prevent infinite recursion
    
    // Process all bindings in this environment
    struct NadaBinding *binding = env->bindings;
    while (binding != NULL) {
        if (binding->value && binding->value->type == NADA_FUNC) {
            // Break circular reference by nulling out environment references
            if (binding->value->data.function.env) {
                // Process the function's environment first (recursive)
                break_env_cycles(binding->value->data.function.env, depth + 1);
                // Then null out the reference
                binding->value->data.function.env = NULL;
            }
        }
        binding = binding->next;
    }
    
    // Also process parent environment recursively
    if (env->parent) {
        break_env_cycles(env->parent, depth + 1);
    }
}

// Cleanup the test environment
static void cleanup_test_env() {
    if (test_env) {
        // Break all circular references recursively
        break_env_cycles(test_env, 0);
        
        // Now we can release the environment
        printf("Releasing test environment with ref count: %d\n", test_env->ref_count);
        nada_env_release(test_env);
        test_env = NULL;
    }
}

static void report_results() {
    printf("\n==== Test Summary ====\n");
    printf("Ran %d tests\n", tests_run);
    printf("Passed: %d\n", tests_passed);
    printf("Failed: %d\n", tests_run - tests_passed);
    printf("========================\n");
}

// Setup test environment with testing functions that work with your implementation
static void setup_test_env(NadaEnv *env) {
    // Instead of defining the test functions directly,
    // load them from the testing library
    char *lib_path = NULL;
    
    // Try to find the testing library
    const char *lib_dirs[] = {
        "src/nadalib/testing.scm",
        "../src/nadalib/testing.scm",
        "../../src/nadalib/testing.scm",
        "../../../src/nadalib/testing.scm",
        "./nadalib/testing.scm",
        "/usr/local/share/nada/lib/testing.scm",
        NULL  // End marker
    };
    
    for (int i = 0; lib_dirs[i] != NULL; i++) {
        FILE *file = fopen(lib_dirs[i], "r");
        if (file) {
            lib_path = strdup(lib_dirs[i]);
            fclose(file);
            break;
        }
    }
    
    if (lib_path) {
        printf("Loading testing library from %s\n", lib_path);
        NadaValue *result = nada_load_file(lib_path, env);
        nada_free(result);
        free(lib_path);
    } else {
        printf("Warning: Could not find testing library. Defining test functions inline.\n");
        
        // Fallback to inline definitions if library not found
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

        // Define the test function
        expr = nada_parse(
            "(define define-test "
            "  (lambda (name body) "
            "    (display \"Test: \") "
            "    (display name) "
            "    (let ((result body)) "
            "      (display \"... \") "
            "      (if (equal? result #t) "
            "          (display \"PASSED\\n\") "
            "          (display \"FAILED\\n\")) "
            "      result)))");

        result = nada_eval(expr, env);
        nada_free(expr);
        nada_free(result);
    }
}

// Run a single test file
static int run_test_file(const char *filename, NadaEnv *env) {
    printf("Running tests from %s\n", filename);

    // Reset error flag
    reset_error_flag();

    // Store current test count
    int tests_before = tests_run;

    // Load and evaluate the file with additional error handling
    NadaValue *result = NULL;
    
    // Set up a signal handler or use setjmp/longjmp for crash protection if needed
    
    // Load and evaluate with careful error handling
    result = nada_load_file(filename, env);
    
    // Check for errors
    int success = !had_evaluation_error && result != NULL;

    // Calculate tests run in this file
    int file_tests = tests_run - tests_before;
    printf("Ran %d tests from %s\n", file_tests, filename);

    // Clean up
    if (result) nada_free(result);

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

    // Add testing functions to environment
    setup_test_env(test_env);

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

    // Register an atexit handler to ensure cleanup happens even on crashes
    atexit(cleanup_test_env);

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

    // We don't need to call cleanup_test_env here - it'll be called by atexit

    return result ? 0 : 1;
}