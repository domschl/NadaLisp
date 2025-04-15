#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "NadaOutput.h"
#include "NadaEval.h"

// Default stdout output handler
static void default_write(const char *str, void *user_data) {
    fputs(str, stdout);
}

static void default_write_value(NadaValue *val, void *user_data) {
    // Format and print value to stdout
    if (val == NULL) {
        fputs("NULL", stdout);
        return;
    }

    switch (val->type) {
    case NADA_NUM: {
        char *str = nada_num_to_string(val->data.number);
        fputs(str, stdout);
        free(str);
        break;
    }
    case NADA_STRING:
        fprintf(stdout, "\"%s\"", val->data.string);
        break;
    case NADA_SYMBOL:
        fputs(val->data.symbol, stdout);
        break;
    case NADA_NIL:
        fputs("()", stdout);
        break;
    case NADA_PAIR:
        putc('(', stdout);
        default_write_value(val->data.pair.car, NULL);

        // Print rest of the list
        NadaValue *rest = val->data.pair.cdr;
        while (rest->type == NADA_PAIR) {
            putc(' ', stdout);
            default_write_value(rest->data.pair.car, NULL);
            rest = rest->data.pair.cdr;
        }

        // Handle improper lists
        if (rest->type != NADA_NIL) {
            fputs(" . ", stdout);
            default_write_value(rest, NULL);
        }

        putc(')', stdout);
        break;
    case NADA_FUNC:
        if (val->data.function.builtin) {
            const char *name = get_builtin_name(val->data.function.builtin);
            if (name) {
                fprintf(stdout, "#<builtin-function:%s>", name);
            } else {
                fputs("#<builtin-function>", stdout);
            }
        } else {
            fputs("#<lambda ", stdout);
            default_write_value(val->data.function.params, NULL);
            putc('>', stdout);
        }
        break;
    case NADA_BOOL:
        fputs(val->data.boolean ? "#t" : "#f", stdout);
        break;
    case NADA_ERROR:
        fprintf(stdout, "Error: %s", val->data.error);
        break;
    }
}

// Global output handler with default implementation
static NadaOutputHandler default_handler = {
    .write = default_write,
    .write_value = default_write_value,
    .user_data = NULL};

NadaOutputHandler *nada_current_output = &default_handler;

void nada_output_init(void) {
    // Use default handler
    nada_current_output = &default_handler;
}

void nada_output_cleanup(void) {
    // Reset to default handler if needed
    nada_current_output = &default_handler;
}

void nada_set_output_handler(NadaOutputHandler *handler) {
    if (handler != NULL) {
        nada_current_output = handler;
    } else {
        nada_current_output = &default_handler;
    }
}

void nada_write_string(const char *str) {
    if (nada_current_output && nada_current_output->write) {
        nada_current_output->write(str, nada_current_output->user_data);
    }
}

void nada_write_format(const char *format, ...) {
    if (!nada_current_output || !nada_current_output->write)
        return;

    va_list args;
    va_start(args, format);

    // First determine buffer size
    va_list args_copy;
    va_copy(args_copy, args);
    int size = vsnprintf(NULL, 0, format, args_copy) + 1;
    va_end(args_copy);

    char *buffer = malloc(size);
    if (buffer) {
        vsnprintf(buffer, size, format, args);
        nada_current_output->write(buffer, nada_current_output->user_data);
        free(buffer);
    }

    va_end(args);
}

void nada_write_value(NadaValue *val) {
    if (nada_current_output && nada_current_output->write_value) {
        nada_current_output->write_value(val, nada_current_output->user_data);
    }
}

// Current output type for Jupyter
static NadaOutputType jupyter_output_type = NADA_OUTPUT_TEXT;

void nada_jupyter_set_output_type(NadaOutputType type) {
    jupyter_output_type = type;
}

NadaOutputType nada_jupyter_get_output_type(void) {
    return jupyter_output_type;
}

// Buffer for capturing Jupyter output
typedef struct {
    char *data;
    size_t capacity;
    size_t length;
} JupyterBuffer;

static JupyterBuffer jupyter_buffer = {NULL, 0, 0};

// Initialize the Jupyter buffer
void nada_jupyter_init_buffer(void) {
    // Free previous buffer if exists
    if (jupyter_buffer.data) {
        free(jupyter_buffer.data);
    }

    // Allocate initial buffer
    jupyter_buffer.capacity = 4096;
    jupyter_buffer.data = malloc(jupyter_buffer.capacity);
    jupyter_buffer.length = 0;

    if (jupyter_buffer.data) {
        jupyter_buffer.data[0] = '\0';
    }
}

// Get the buffer content
const char *nada_jupyter_get_buffer(void) {
    return jupyter_buffer.data ? jupyter_buffer.data : "";
}

// Clear the Jupyter buffer
void nada_jupyter_clear_buffer(void) {
    if (jupyter_buffer.data) {
        jupyter_buffer.data[0] = '\0';
        jupyter_buffer.length = 0;
    }
    // Reset the output type as well
    jupyter_output_type = NADA_OUTPUT_TEXT;
}

// Append to the Jupyter buffer
static void jupyter_write(const char *str, void *user_data) {
    if (!jupyter_buffer.data || !str) return;

    size_t len = strlen(str);

    // Ensure buffer has enough space
    if (jupyter_buffer.length + len + 1 > jupyter_buffer.capacity) {
        size_t new_capacity = jupyter_buffer.capacity * 2 + len;
        char *new_buffer = realloc(jupyter_buffer.data, new_capacity);

        if (!new_buffer) return;

        jupyter_buffer.data = new_buffer;
        jupyter_buffer.capacity = new_capacity;
    }

    // Append the string
    strcpy(jupyter_buffer.data + jupyter_buffer.length, str);
    jupyter_buffer.length += len;
}

// Custom output handler for Jupyter
static NadaOutputHandler jupyter_handler = {
    .write = jupyter_write,
    .write_value = default_write_value,  // Reuse default formatter
    .user_data = NULL};

// Set up Jupyter output
void nada_jupyter_use_output(void) {
    nada_set_output_handler(&jupyter_handler);
    nada_jupyter_init_buffer();
    // Start with text output mode
    jupyter_output_type = NADA_OUTPUT_TEXT;
}

// Free Jupyter buffer resources
void nada_jupyter_cleanup(void) {
    if (jupyter_buffer.data) {
        free(jupyter_buffer.data);
        jupyter_buffer.data = NULL;
    }
    jupyter_buffer.capacity = 0;
    jupyter_buffer.length = 0;
    jupyter_output_type = NADA_OUTPUT_TEXT;
}