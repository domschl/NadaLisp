#include "NadaError.h"
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

// Global error handler
static NadaErrorHandler current_handler = NULL;
static void *current_user_data = NULL;

// Global error state
static NadaErrorType current_error_type = NADA_ERROR_NONE;
static char current_error_message[1024] = {0};

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

// Get the current error handler
NadaErrorHandler nada_get_error_handler() {
    return current_handler;
}

// Get the current error code
NadaErrorType nada_get_error_code() {
    return current_error_type;
}

// Get the current error message
const char* nada_get_error_message() {
    return current_error_message;
}

// Clear the current error state
void nada_clear_error() {
    current_error_type = NADA_ERROR_NONE;
    current_error_message[0] = '\0';
}

// Report an error
void nada_report_error(NadaErrorType type, const char *format, ...) {
    // Save the error type
    current_error_type = type;
    
    // Format the error message
    va_list args;
    va_start(args, format);
    vsnprintf(current_error_message, sizeof(current_error_message), format, args);
    va_end(args);

    // Call the error handler if one is set
    if (current_handler) {
        current_handler(type, current_error_message, current_user_data);
    } else {
        // Use default handler
        default_error_handler(type, current_error_message, NULL);
    }
}

// Report a syntax error
void nada_report_syntax_error(const char *filename, int line_number, const char *line_content, int position, const char *format, ...) {
    va_list args;
    char buffer[1024];

    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    fprintf(stderr, "Syntax error in %s (line %d): %s\n", filename, line_number, buffer);

    // Print the offending line
    if (line_content) {
        fprintf(stderr, "%s\n", line_content);

        // Print a caret pointer to the error position
        if (position >= 0) {
            for (int i = 0; i < position; i++) {
                fputc(' ', stderr);
            }
            fprintf(stderr, "^\n");
        }
    }
}