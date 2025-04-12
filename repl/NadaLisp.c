#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "NadaValue.h"
#include "NadaParser.h"
#include "NadaEval.h"
#include "NadaConfig.h"

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

void print_usage() {
    printf("Usage: nada [-n] [-c expr | -e expr | filename]\n");
    printf("  -n: do not load the standard libraries\n");
    printf("  -e expr: interpret expr as Scheme expression, evaluate it, exit\n");
    printf("  -c expr: interpret expr as textual algebraic expression, evaluate it, exit\n");
    printf("  If neither -e nor -c is given, expr is interpreted as a Scheme filename\n");
}

int main(int argc, char *argv[]) {
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
                printf("Error: -e requires an expression\n");
                print_usage();
                nada_cleanup_env(global_env);
                return 1;
            }
        } else if (strcmp(argv[i], "-c") == 0) {
            eval_algebraic = 1;
            // Get the expression from the next argument
            if (i + 1 < argc) {
                expression = argv[++i];
            } else {
                printf("Error: -c requires an expression\n");
                print_usage();
                nada_cleanup_env(global_env);
                return 1;
            }
        } else if (argv[i][0] == '-') {
            printf("Unknown option: %s\n", argv[i]);
            print_usage();
            nada_cleanup_env(global_env);
            return 1;
        } else {
            // Non-flag argument should be a filename
            if (!eval_scheme && !eval_algebraic) {
                expression = argv[i];
            } else {
                printf("Error: Unexpected argument: %s\n", argv[i]);
                print_usage();
                nada_cleanup_env(global_env);
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
        NadaValue *expr = nada_parse(expression);
        if (expr) {
            NadaValue *result = nada_eval(expr, global_env);
            nada_print(result);
            printf("\n");
            nada_free(expr);
            nada_free(result);
        } else {
            printf("Error parsing Scheme expression\n");
            nada_cleanup_env(global_env);
            return 1;
        }
    } else if (eval_algebraic) {
        // Evaluate algebraic expression
        char buffer[10240];
        snprintf(buffer, sizeof(buffer), "(eval-algebraic \"%s\")", expression);

        NadaValue *expr = nada_parse(buffer);
        if (expr) {
            NadaValue *result = nada_eval(expr, global_env);
            nada_print(result);
            printf("\n");
            nada_free(expr);
            nada_free(result);
        } else {
            printf("Error parsing algebraic expression\n");
            nada_cleanup_env(global_env);
            return 1;
        }
    } else if (expression) {
        // File mode: load the specified file
        NadaValue *result = nada_load_file(expression, global_env);
        if (result) {
            nada_free(result);
        } else {
            printf("Error loading file: %s\n", expression);
            nada_cleanup_env(global_env);
            return 1;
        }
    } else {
        // Interactive mode: run the REPL
        run_repl();
    }

    nada_cleanup_env(global_env);
    return 0;
}