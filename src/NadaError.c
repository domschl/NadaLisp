#include "NadaError.h"
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

// Global error handler
static NadaErrorHandler current_handler = NULL;
static void *current_user_data = NULL;

// Default error handler (just prints to stderr)
static void default_error_handler(NadaErrorType type, const char *message, void *user_data) {
    fprintf(stderr, "Error: %s\n", message);
}

// Set the error handler
void nada_set_error_handler(NadaErrorHandler handler, void *user_data) {
    current_handler = handler;
    current_user_data = user_data;
}

// Clear the error handler (revert to default)
void nada_clear_error_handler() {
    current_handler = NULL;
    current_user_data = NULL;
}

// Report an error
void nada_report_error(NadaErrorType type, const char *format, ...) {
    // Format the error message
    char message[1024];
    va_list args;
    va_start(args, format);
    vsnprintf(message, sizeof(message), format, args);
    va_end(args);

    // Call the error handler if one is set
    if (current_handler) {
        current_handler(type, message, current_user_data);
    } else {
        // Use default handler
        default_error_handler(type, message, NULL);
    }
}