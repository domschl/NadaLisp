#ifndef NADA_ERROR_H
#define NADA_ERROR_H

#include <stdio.h>

// Define error type enum
typedef enum {
    NADA_ERROR_NONE,
    NADA_ERROR_SYNTAX,
    NADA_ERROR_UNDEFINED_SYMBOL,
    NADA_ERROR_TYPE_ERROR,
    NADA_ERROR_INVALID_ARGUMENT,  // Changed from NADA_ERROR_ARGUMENT_ERROR
    NADA_ERROR_RUNTIME_ERROR,
    // Add more error types as needed
} NadaErrorType;

// Error handler callback type
typedef void (*NadaErrorHandler)(NadaErrorType type, const char *message, void *user_data);

// Set the error handler
void nada_set_error_handler(NadaErrorHandler handler, void *user_data);

// Clear the error handler
void nada_clear_error_handler();

// Report an error (used internally by the library)
void nada_report_error(NadaErrorType type, const char *format, ...);

#endif  // NADA_ERROR_H