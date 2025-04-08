#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>
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

// Process a file containing Lisp code
void process_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Error: Unable to open file '%s'\n", filename);
        return;
    }

    char buffer[10240] = {0};  // Much larger buffer
    char line[1024];
    int paren_balance = 0;
    int in_string = 0;

    while (fgets(line, sizeof(line), file)) {
        // Handle comments
        char *comment = strchr(line, ';');
        if (comment) *comment = '\0';

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
            NadaValue *expr = nada_parse(buffer);
            NadaValue *result = nada_eval(expr, global_env);

            nada_free(expr);
            nada_free(result);

            // Reset buffer for next expression
            buffer[0] = '\0';
        }
    }

    fclose(file);
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

int main(int argc, char *argv[]) {
    // Initialize the global environment
    global_env = nada_create_standard_env();

    // Register cleanup
    atexit(nada_cleanup);

    if (argc > 1) {
        // File mode: process the specified file
        process_file(argv[1]);
    } else {
        // Interactive mode: use readline for REPL
        run_repl();
    }

    return 0;
}