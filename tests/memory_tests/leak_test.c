// tests/memory_tests/leak_test.c
#include "../../src/NadaEval.h"
#include "../../src/NadaParser.h"
#include "../../src/NadaValue.h"
#include "../../src/NadaError.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>  // For getcwd()

// Error handler callback
static void error_handler(NadaErrorType type, const char *message, void *user_data) {
    // Print the error message
    if (! nada_is_global_silent_symbol_lookup()) {
        // Do NOT error out if we are in silent mode, used for testing handling of undefined symbols
        fprintf(stderr, "Leak-Error-Handler: %s\n", message);
    } else {
        printf("Suppressing lookup-error: %s\n", message);
    }
}

// Function to load standard libraries
static void load_libraries(NadaEnv *env) {
    // Try multiple potential library locations
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

    printf("Searching for libraries from working directory: %s\n", cwd_buffer);

    // Try each potential location
    for (int i = 0; lib_dirs[i] != NULL; i++) {
        dir = opendir(lib_dirs[i]);
        if (dir) {
            printf("Found library directory: %s\n", lib_dirs[i]);
            found_index = i;
            break;
        }
    }

    if (found_index < 0) {
        printf("Note: No library directory found. Tests may fail if they depend on libraries.\n");
        return;
    }

    // Use the actual found directory path for loading
    const char *found_dir = lib_dirs[found_index];
    printf("Loading libraries from %s...\n", found_dir);

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        // Skip directories and non-.scm files
        if (entry->d_type == DT_DIR) continue;

        const char *filename = entry->d_name;
        size_t len = strlen(filename);

        // Check for .scm extension
        if (len > 4 && strcmp(filename + len - 4, ".scm") == 0) {
            char full_path[1024];
            snprintf(full_path, sizeof(full_path), "%s/%s", found_dir, filename);

            printf("  Loading %s\n", filename);
            NadaValue *result = nada_load_file(full_path, env);
            nada_free(result);
        }
    }

    closedir(dir);
    printf("Libraries loaded.\n");
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <script_file>\n", argv[0]);
        return 1;
    }

    // Set up the error handler [don't, causes problems with intentional undefs]
    // nada_set_error_handler(error_handler, NULL);

    // Create a standard environment
    NadaEnv *env = nada_create_standard_env();
    
    // Load standard libraries
    load_libraries(env);
    
    printf("Running memory test on %s\n", argv[1]);

    nada_set_silent_symbol_lookup(1);  // Suppress symbol lookup errors
    // Use the proper file loading function to handle multi-line expressions
    NadaValue *result = nada_load_file(argv[1], env);
    nada_set_silent_symbol_lookup(0);  // Restore symbol lookup errors
    
    // Clean up
    nada_free(result);
    nada_env_free(env);

    return 0;
}