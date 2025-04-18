#ifndef NADA_OUTPUT_H
#define NADA_OUTPUT_H

#include <stdio.h>
#include "NadaValue.h"

typedef struct NadaOutputHandler NadaOutputHandler;

// Output handler methods
typedef void (*NadaOutputFn)(const char *str, void *user_data);
typedef void (*NadaValueOutputFn)(NadaValue *val, void *user_data);

struct NadaOutputHandler {
    NadaOutputFn write;             // Write raw text
    NadaValueOutputFn write_value;  // Write formatted value
    void *user_data;                // Context for output functions
};

// Global output handler (default: stdout)
extern NadaOutputHandler *nada_current_output;

// Initialize default output handler
void nada_output_init(void);

// Clean up output handler
void nada_output_cleanup(void);

// Set custom output handler
void nada_set_output_handler(NadaOutputHandler *handler);

// Core output functions
void nada_write_string(const char *str);
void nada_write_format(const char *format, ...);
void nada_write_value(NadaValue *val);

// String conversion (returns newly allocated string)
// char *nada_value_to_string(NadaValue *val);

// Output types
typedef enum {
    NADA_OUTPUT_TEXT = 0,
    NADA_OUTPUT_MARKDOWN = 1,
    NADA_OUTPUT_HTML = 2
} NadaOutputType;

// Set output mode for jupyter
void nada_jupyter_set_output_type(NadaOutputType type);
void nada_jupyter_init_buffer(void);
const char *nada_jupyter_get_buffer(void);
void nada_jupyter_clear_buffer(void);
void nada_jupyter_use_output(void);
void nada_jupyter_cleanup(void);

#endif  // NADA_OUTPUT_H