#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>  // For getcwd()
#include <readline/readline.h>
#include <readline/history.h>
#include <dirent.h>  // Include for directory handling
#include "NadaValue.h"
#include "NadaParser.h"
#include "NadaEval.h"

// Global environment
static NadaEnv *global_env;

// Process a single line of Lisp code
void process_line(const char *line) {
    printf("Processing: %s\n", line);

    // Initialize tokenizer
    Tokenizer tokenizer;
    tokenizer_init(&tokenizer, line);

    if (get_next_token(&tokenizer)) {
        // Parse the expression
        NadaValue *expr = parse_expr(&tokenizer);

        printf("Parsed: ");
        nada_print(expr);
        printf("\n");

        // Evaluate the expression
        NadaValue *result = nada_eval(expr, global_env);

        // Print the result
        printf("Result: ");
        nada_print(result);
        printf("\n");

        // Free the memory
        nada_free(expr);
        nada_free(result);
    }
}

// Run an interactive REPL (Read-Eval-Print Loop)
void run_repl(void) {
    printf("NadaLisp REPL (Ctrl+D to exit)\n");
    nada_memory_reset();

    char buffer[10240] = {0};
    int paren_balance = 0;
    int in_string = 0;
    char prompt[32] = "nada> ";

    while (1) {
        char *line = readline(prompt);
        if (!line) break;  // Ctrl+D

        if (strlen(line) == 0) {
            free(line);
            continue;
        }

        // Handle comments
        char *comment = strchr(line, ';');
        if (comment) *comment = '\0';

        // Skip empty lines
        if (strlen(line) == 0) {
            free(line);
            continue;
        }

        // Add to history only if we're at the start of an expression
        if (buffer[0] == '\0') {
            add_history(line);
        }

        // Append to the buffer
        strcat(buffer, line);
        strcat(buffer, " ");  // Add space for readability

        // Count parentheses and track strings
        for (char *p = line; *p; p++) {
            if (*p == '"') in_string = !in_string;
            if (!in_string) {
                if (*p == '(')
                    paren_balance++;
                else if (*p == ')')
                    paren_balance--;
            }
        }

        free(line);

        // If balanced, process the expression
        if (paren_balance == 0) {
            if (strlen(buffer) > 0) {
                NadaValue *expr = nada_parse(buffer);
                NadaValue *result = nada_eval(expr, global_env);

                nada_print(result);
                printf("\n");

                nada_free(expr);
                nada_free(result);
                // nada_memory_report();
            }
            buffer[0] = '\0';
            strcpy(prompt, "nada> ");
        } else {
            // Change prompt to show we're awaiting more input
            strcpy(prompt, "...... ");
        }
    }

    printf("\nGoodbye!\n");
}

// Fix environment handling
void nada_cleanup() {
    // Free the global environment (which will free all values in it)
    if (global_env != NULL) {
        nada_env_free(global_env);
        global_env = NULL;
    }

    // Print final memory report
    // nada_memory_report();
}

// Fix the path composition in load_libraries
void load_libraries(NadaEnv *env) {
    // Try multiple potential library locations
    const char *lib_dirs[] = {
        "src/nadalib",                // When run from project root
        "../src/nadalib",             // When run from build directory
        "./nadalib",                  // When run from src directory
        "/usr/local/share/nada/lib",  // System-wide installation
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
            found_index = i;  // Store the index
            break;
        }

        // Show the full path we tried
        if (lib_dirs[i][0] == '/') {
            // Absolute path
            printf("Tried library path: %s (not found)\n", lib_dirs[i]);
        } else {
            // Relative path
            printf("Tried library path: %s/%s (not found)\n", cwd_buffer, lib_dirs[i]);
        }
    }

    if (found_index < 0) {
        printf("Note: No library directory found. Libraries not loaded.\n");
        printf("Create the directory 'src/nadalib' and add .scm files there.\n");
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
            // Use the CORRECT found directory path
            char full_path[1024];
            snprintf(full_path, sizeof(full_path), "%s/%s", found_dir, filename);
ß
            printf("  Loading %s\n", filename);
            NadaValue *result = nada_load_file(full_path, global_env);
            nada_free(result);
        }
    }

    closedir(dir);
    printf("Libraries loaded successfully.\n");
}

// Update the main function to call nada_load_file directly
int main(int argc, char *argv[]) {
    // Initialize the global environment
    global_env = nada_create_standard_env();

    // Register cleanup
    atexit(nada_cleanup);

    // Load libraries
    load_libraries(global_env);

    if (argc > 1) {
        // File mode: load the specified file using nada_load_file
        NadaValue *result = nada_load_file(argv[1], global_env);
        nada_free(result);
    } else {
        // Interactive mode: run the REPL
        run_repl();
    }

    return 0;
}