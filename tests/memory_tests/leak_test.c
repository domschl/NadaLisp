// tests/memory_tests/leak_test.c
#include "NadaEval.h"
#include "NadaParser.h"
#include "NadaValue.h"
#include "NadaError.h"
#include "NadaConfig.h"
#include "NadaOutput.h"  // Add the new output header
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>  // For getcwd()

// Error handler callback
static void error_handler(NadaErrorType type, const char *message, void *user_data) {
    // Print the error message
    if (!nada_is_global_silent_symbol_lookup()) {
        // Do NOT error out if we are in silent mode, used for testing handling of undefined symbols
        nada_write_format("Leak-Error-Handler: %s\n", message);
    } else {
        nada_write_string("Suppressing lookup-error: ");
        nada_write_string(message);
        nada_write_string("\n");
    }
}

static NadaEnv *global_env = NULL;

int main(int argc, char *argv[]) {
    // Initialize output system
    nada_output_init();

    if (argc < 2) {
        nada_write_format("Usage: %s <script_file>\n", argv[0]);
        nada_output_cleanup();
        return 1;
    }

    // Set up the error handler [don't, causes problems with intentional undefs]
    // nada_set_error_handler(error_handler, NULL);

    // Create a standard environment
    global_env = nada_create_standard_env();

    // Load standard libraries
    nada_load_libraries(global_env);

    nada_write_format("Running memory test on %s\n", argv[1]);

    // nada_set_silent_symbol_lookup(1);  // Suppress symbol lookup errors
    //  Use the proper file loading function to handle multi-line expressions
    NadaValue *result = nada_load_file(argv[1], global_env);
    // nada_set_silent_symbol_lookup(0);  // Restore symbol lookup errors

    // Clean up
    nada_free(result);
    nada_cleanup_env(global_env);

    // Clean up output system before exiting
    nada_output_cleanup();

    return 0;
}