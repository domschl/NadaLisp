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

    char line[1024];
    while (fgets(line, sizeof(line), file)) {
        // Remove trailing newline
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') {
            line[len - 1] = '\0';
        }

        process_line(line);
    }

    fclose(file);
}

// Run an interactive REPL (Read-Eval-Print Loop)
void run_repl(void) {
    printf("NadaLisp REPL (Ctrl+D to exit)\n");
    nada_memory_reset();  // Reset counters at startup

    char *line;
    while ((line = readline("nada> ")) != NULL) {
        if (strlen(line) > 0) {
            add_history(line);

            NadaValue *expr = nada_parse(line);
            NadaValue *result = nada_eval(expr, global_env);

            // Print result
            nada_print(result);
            printf("\n");

            // Ensure both expr and result are freed
            nada_free(expr);
            nada_free(result);
        }
        free(line);  // Don't forget to free readline's line
        nada_memory_report();
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
    nada_memory_report();
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