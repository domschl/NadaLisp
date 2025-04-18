#ifndef NADA_ERROR_H
#define NADA_ERROR_H

#include <stdio.h>
#include <NadaValue.h>

// Define error type enum
typedef enum {
    NADA_ERROR_NONE,              // No error
    NADA_ERROR_SYNTAX,            // Syntax error in parsing
    NADA_ERROR_INVALID_ARGUMENT,  // Invalid argument to function
    NADA_ERROR_TYPE_ERROR,        // Type error
    NADA_ERROR_UNDEFINED_SYMBOL,  // Undefined symbol
    NADA_ERROR_MEMORY,            // Memory allocation error
    NADA_ERROR_DIVISION_BY_ZERO,  // Division by zero
    NADA_ERROR_OUT_OF_MEMORY,     // Out of memory
    // Add more error types as needed
} NadaErrorType;

// Error handler callback type
typedef void (*NadaErrorHandler)(NadaErrorType type, const char *message, void *user_data);

// Get the current error handler
NadaErrorHandler nada_get_error_handler();

// Set the error handler
void nada_set_error_handler(NadaErrorHandler handler, void *user_data);

// Clear the error handler
void nada_clear_error_handler();

// Report an error (used internally by the library)
void nada_report_error(NadaErrorType type, const char *format, ...);

// Add this prototype:
void nada_report_syntax_error(const char *filename, int line_number, const char *line_content, int position, const char *format, ...);

// Get the current error code
NadaErrorType nada_get_error_code();

// Get the current error message
const char *nada_get_error_message();

// Clear the current error state
void nada_clear_error();

void *nada_get_user_data(void);

// Another global error state function, used by parse_eval_multi()
int nada_check_error(void);
NadaValue *nada_get_error_value(void);

#endif  // NADA_ERROR_H