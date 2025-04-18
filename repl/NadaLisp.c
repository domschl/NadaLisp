#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "NadaValue.h"
#include "NadaParser.h"
#include "NadaEval.h"
#include "NadaConfig.h"
#include "NadaOutput.h"  // Include the new output header

// Global environment
static NadaEnv *global_env;

// Add this helper function before run_repl()
static void clean_buffer_whitespace(char *buffer) {
    if (!buffer || buffer[0] == '\0') return;  // Handle empty buffer

    int in_string = 0;
    int i = 0, j = 0;

    // Compress the buffer in-place
    while (buffer[i]) {
        // Track string state (toggle when we see a quotation mark)
        if (buffer[i] == '"' && (i == 0 || buffer[i - 1] != '\\')) {
            in_string = !in_string;
        }

        // Handle spaces: skip consecutive spaces unless in string
        if (buffer[i] == ' ' && !in_string) {
            // Add one space, then skip any following spaces
            buffer[j++] = ' ';
            while (buffer[i + 1] == ' ')
                i++;
        } else {
            // Copy the character
            buffer[j++] = buffer[i];
        }

        i++;
    }

    // Null-terminate the potentially shortened buffer
    buffer[j] = '\0';
}

// Run an interactive REPL (Read-Eval-Print Loop)
void run_repl(void) {
    // Initialize output system
    nada_output_init();

    nada_write_string("NadaLisp REPL (Ctrl+D to exit)\n");
    nada_memory_reset();

    // Use dynamic allocation instead of fixed buffer
    size_t buffer_size = 1024;  // Initial size
    char *buffer = malloc(buffer_size);
    if (!buffer) {
        nada_write_format("Memory allocation failed\n");
        return;
    }
    buffer[0] = '\0';

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

        // Resize buffer if needed
        size_t current_len = strlen(buffer);
        size_t line_len = strlen(line);
        size_t required_size = current_len + line_len + 2;  // +2 for space and null terminator

        if (required_size > buffer_size) {
            // Double the buffer size or increase to required size, whichever is larger
            size_t new_size = buffer_size * 2;
            if (new_size < required_size) {
                new_size = required_size;
            }

            char *new_buffer = realloc(buffer, new_size);
            if (!new_buffer) {
                nada_write_format("Memory reallocation failed\n");
                free(buffer);
                free(line);
                return;
            }

            buffer = new_buffer;
            buffer_size = new_size;
        }

        // Append to the buffer
        strcat(buffer, line);
        strcat(buffer, " ");  // Add space for readability

        free(line);

        // Check balance using the library function
        int error_pos = -1;
        int paren_balance = nada_validate_parentheses(buffer, &error_pos);

        // If balanced, process the expression
        if (paren_balance == 0) {
            if (strlen(buffer) > 0) {
                NadaValue *result = nada_parse_eval_multi(buffer, global_env);

                // Print the result using the output system
                nada_write_value(result);
                nada_write_string("\n");

                nada_free(result);
                // Remove double spaces in buffer before adding to history
                clean_buffer_whitespace(buffer);
                add_history(buffer);  // Add to history only if balanced
            }
            buffer[0] = '\0';  // Reset buffer but keep allocated memory

            strcpy(prompt, "nada> ");
        } else if (paren_balance < 0) {
            // Unbalanced closing parenthesis - syntax error
            nada_write_format("Error: Unexpected closing bracket at position %d\n", error_pos);
            buffer[0] = '\0';  // Reset buffer
            strcpy(prompt, "nada> ");
        } else {
            // Change prompt to show we're awaiting more input
            strcpy(prompt, "...... ");
        }
    }

    // Clean up
    free(buffer);
    nada_write_string("\nGoodbye!\n");

    // Clean up output system
    nada_output_cleanup();
}

void print_usage() {
    nada_write_string("Usage: nada [-n] [-c expr | -e expr | filename]\n");
    nada_write_string("  -n: do not load the standard libraries\n");
    nada_write_string("  -e expr: interpret expr as Scheme expression, evaluate it, exit\n");
    nada_write_string("  -c expr: interpret expr as textual algebraic expression, evaluate it, exit\n");
    nada_write_string("  If neither -e nor -c is given, expr is interpreted as a Scheme filename\n");
}

int main(int argc, char *argv[]) {
    // Initialize output system at program start
    nada_output_init();

    // Initialize the global environment
    global_env = nada_create_standard_env();

    // Parse command-line arguments
    int load_libs = 1;  // Default: load standard libraries
    int eval_scheme = 0;
    int eval_algebraic = 0;
    char *expression = NULL;

    // Process all arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-n") == 0) {
            load_libs = 0;
        } else if (strcmp(argv[i], "-e") == 0) {
            eval_scheme = 1;
            // Get the expression from the next argument
            if (i + 1 < argc) {
                expression = argv[++i];
            } else {
                nada_write_format("Error: -e requires an expression\n");
                print_usage();
                nada_cleanup_env(global_env);
                nada_output_cleanup();
                return 1;
            }
        } else if (strcmp(argv[i], "-c") == 0) {
            eval_algebraic = 1;
            // Get the expression from the next argument
            if (i + 1 < argc) {
                expression = argv[++i];
            } else {
                nada_write_format("Error: -c requires an expression\n");
                print_usage();
                nada_cleanup_env(global_env);
                nada_output_cleanup();
                return 1;
            }
        } else if (argv[i][0] == '-') {
            nada_write_format("Unknown option: %s\n", argv[i]);
            print_usage();
            nada_cleanup_env(global_env);
            nada_output_cleanup();
            return 1;
        } else {
            // Non-flag argument should be a filename
            if (!eval_scheme && !eval_algebraic) {
                expression = argv[i];
            } else {
                nada_write_format("Error: Unexpected argument: %s\n", argv[i]);
                print_usage();
                nada_cleanup_env(global_env);
                nada_output_cleanup();
                return 1;
            }
            break;
        }
    }

    // Load libraries if not disabled
    if (load_libs) {
        nada_load_libraries(global_env);
    }

    // Execute based on the parsed options
    if (eval_scheme) {
        // Evaluate Scheme expression
        NadaValue *result = nada_parse_eval_multi(expression, global_env);
        nada_write_value(result);
        nada_write_string("\n");

        // Check for errors and exit with non-zero status if there was an error
        int exit_code = 0;
        if (nada_is_error(result)) {
            exit_code = 1;
        }

        nada_free(result);

        nada_output_cleanup();
        nada_cleanup_env(global_env);
        return exit_code;
    } else if (eval_algebraic) {
        // Evaluate algebraic expression
        char buffer[10240];
        snprintf(buffer, sizeof(buffer), "(eval-algebraic \"%s\")", expression);

        NadaValue *result = nada_parse_eval_multi(buffer, global_env);
        nada_write_value(result);
        nada_write_string("\n");

        // Check for errors and exit with non-zero status if there was an error
        int exit_code = 0;
        if (nada_is_error(result)) {
            exit_code = 1;
        }

        nada_free(result);

        nada_output_cleanup();
        nada_cleanup_env(global_env);
        return exit_code;
    } else if (expression) {
        // File mode: load the specified file
        NadaValue *result = nada_load_file(expression, global_env);
        if (result) {
            nada_free(result);
        } else {
            nada_write_format("Error loading file: %s\n", expression);
            nada_cleanup_env(global_env);
            nada_output_cleanup();
            return 1;
        }
    } else {
        // Interactive mode: run the REPL
        run_repl();
    }

    nada_cleanup_env(global_env);
    nada_output_cleanup();
    return 0;
}