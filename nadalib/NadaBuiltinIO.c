#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "NadaEval.h"
#include "NadaError.h"
#include "NadaParser.h"
#include "NadaBuiltinIO.h"

// Built-in function: save-environment
NadaValue *builtin_save_environment(NadaValue *args, NadaEnv *env) {
    if (nada_is_nil(args) || !nada_is_nil(nada_cdr(args))) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "save-environment requires exactly one filename argument");
        return nada_create_bool(0);
    }

    NadaValue *filename_arg = nada_eval(nada_car(args), env);
    if (filename_arg->type != NADA_STRING) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "save-environment requires a string filename");
        nada_free(filename_arg);
        return nada_create_bool(0);
    }

    FILE *file = fopen(filename_arg->data.string, "w");
    if (!file) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "could not open file %s for writing", filename_arg->data.string);
        nada_free(filename_arg);
        return nada_create_bool(0);
    }

    nada_serialize_env(env, file);
    fclose(file);

    nada_free(filename_arg);
    return nada_create_bool(1);  // Return true for success
}

// Built-in function: load-file
NadaValue *builtin_load_file(NadaValue *args, NadaEnv *env) {
    // Validate arguments
    if (nada_is_nil(args) || !nada_is_nil(nada_cdr(args))) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "load-file requires exactly one filename argument");
        return nada_create_bool(0);
    }

    // Get and validate filename
    NadaValue *filename_arg = nada_eval(nada_car(args), env);
    if (filename_arg->type != NADA_STRING) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "load-file requires a string filename");
        nada_free(filename_arg);
        return nada_create_bool(0);
    }

    // Open file
    FILE *file = fopen(filename_arg->data.string, "r");
    if (!file) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "could not open file %s for reading",
                          filename_arg->data.string);
        nada_free(filename_arg);
        return nada_create_bool(0);
    }

    // Read the entire file content in one go
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    rewind(file);

    // Allocate buffer with extra space for a null terminator
    char *file_content = malloc(file_size + 1);
    if (!file_content) {
        nada_report_error(NADA_ERROR_MEMORY, "failed to allocate memory for file content");
        fclose(file);
        nada_free(filename_arg);
        return nada_create_bool(0);
    }

    // Read the entire file
    size_t bytes_read = fread(file_content, 1, file_size, file);
    file_content[bytes_read] = '\0';
    fclose(file);

    // Now process the file content
    char *current_pos = file_content;
    char *buffer = malloc(bytes_read + 1);
    if (!buffer) {
        nada_report_error(NADA_ERROR_MEMORY, "failed to allocate memory for parsing buffer");
        free(file_content);
        nada_free(filename_arg);
        return nada_create_bool(0);
    }

    NadaValue *last_result = nada_create_nil();

    // Process the file content, one expression at a time
    while (*current_pos) {
        // Skip whitespace and comments
        while (*current_pos && (isspace(*current_pos) || *current_pos == ';')) {
            if (*current_pos == ';') {
                // Skip to end of line
                while (*current_pos && *current_pos != '\n')
                    current_pos++;
            }
            if (*current_pos)
                current_pos++;
        }

        // If we reached the end, we're done
        if (!*current_pos)
            break;

        // Extract a single, complete expression
        int depth = 0;
        int in_string = 0;
        int escaped = 0;
        int pos = 0;

        // Read until we have a complete expression
        char c;
        while ((c = *current_pos)) {
            buffer[pos++] = c;
            current_pos++;

            if (escaped) {
                escaped = 0;
                continue;
            }

            if (c == '\\' && in_string) {
                escaped = 1;
            } else if (c == '"' && !escaped) {
                in_string = !in_string;
            } else if (!in_string) {
                if (c == '(') {
                    depth++;
                } else if (c == ')') {
                    depth--;
                    // Complete expression found
                    if (depth == 0 && pos > 0) {
                        buffer[pos] = '\0';

                        // Parse and evaluate the expression
                        NadaValue *expr = nada_parse(buffer);
                        if (expr) {
                            nada_free(last_result);
                            last_result = nada_eval(expr, env);
                            nada_free(expr);
                        }

                        pos = 0;
                        break;
                    }
                } else if (depth == 0 && !isspace(c) && c != ';') {
                    // This is an atom outside parentheses (like a number or symbol)
                    // Read until whitespace or comment
                    while (*current_pos && !isspace(*current_pos) && *current_pos != ';' &&
                           *current_pos != '(' && *current_pos != ')') {
                        buffer[pos++] = *current_pos++;
                    }

                    buffer[pos] = '\0';

                    // Parse and evaluate the atom
                    NadaValue *expr = nada_parse(buffer);
                    if (expr) {
                        nada_free(last_result);
                        last_result = nada_eval(expr, env);
                        nada_free(expr);
                    }

                    pos = 0;
                    break;
                }
            }

            // Safety check to prevent buffer overflow
            if (pos >= bytes_read) {
                nada_report_error(NADA_ERROR_SYNTAX, "expression too large in file %s",
                                  filename_arg->data.string);
                break;
            }
        }
    }

    // Clean up
    free(buffer);
    free(file_content);
    nada_free(filename_arg);

    return last_result;
}

NadaValue *nada_load_file(const char *filename, NadaEnv *env) {
    // Create load-file arguments: filename as a string
    NadaValue *string_arg = nada_create_string(filename);
    NadaValue *nil_arg = nada_create_nil();
    NadaValue *args = nada_cons(string_arg, nil_arg);

    // Free the intermediate values that have been copied by nada_cons
    nada_free(string_arg);
    nada_free(nil_arg);

    // Call the built-in function
    NadaValue *result = builtin_load_file(args, env);

    // Clean up
    nada_free(args);

    return result;
}

// read-line: Read a line from console  XXX fixed buffer!
NadaValue *builtin_read_line(NadaValue *args, NadaEnv *env) {
    if (!nada_is_nil(args)) {
        // If prompt is provided, display it
        NadaValue *prompt = nada_eval(nada_car(args), env);
        if (prompt->type == NADA_STRING) {
            printf("%s", prompt->data.string);
        } else {
            nada_print(prompt);
        }
        nada_free(prompt);
    }

    char buffer[1024];
    if (fgets(buffer, sizeof(buffer), stdin)) {
        // Remove trailing newline
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
        }

        return nada_create_string(buffer);
    }

    return nada_create_string("");  // Empty string if read failed
}

// read-file: Read a file into a string
NadaValue *builtin_read_file(NadaValue *args, NadaEnv *env) {
    if (nada_is_nil(args) || !nada_is_nil(nada_cdr(args))) {
        fprintf(stderr, "Error: read-file requires exactly 1 argument\n");
        return nada_create_nil();
    }

    NadaValue *path_val = nada_eval(nada_car(args), env);
    if (path_val->type != NADA_STRING) {
        fprintf(stderr, "Error: read-file requires a string argument\n");
        nada_free(path_val);
        return nada_create_nil();
    }

    FILE *file = fopen(path_val->data.string, "rb");
    if (!file) {
        fprintf(stderr, "Error: Could not open file %s\n", path_val->data.string);
        nada_free(path_val);
        return nada_create_nil();
    }

    // Get file size
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Allocate buffer
    char *buffer = malloc(size + 1);
    if (!buffer) {
        fprintf(stderr, "Error: Out of memory\n");
        fclose(file);
        nada_free(path_val);
        return nada_create_nil();
    }

    // Read the file
    size_t bytes_read = fread(buffer, 1, size, file);
    fclose(file);

    buffer[bytes_read] = '\0';

    nada_free(path_val);
    NadaValue *result = nada_create_string(buffer);
    free(buffer);

    return result;
}

// write-file: Write a string to a file
NadaValue *builtin_write_file(NadaValue *args, NadaEnv *env) {
    if (nada_is_nil(args) || nada_is_nil(nada_cdr(args)) ||
        !nada_is_nil(nada_cdr(nada_cdr(args)))) {
        fprintf(stderr, "Error: write-file requires exactly 2 arguments\n");
        return nada_create_bool(0);
    }

    NadaValue *path_val = nada_eval(nada_car(args), env);
    if (path_val->type != NADA_STRING) {
        fprintf(stderr, "Error: write-file requires a string as first argument\n");
        nada_free(path_val);
        return nada_create_bool(0);
    }

    NadaValue *content_val = nada_eval(nada_car(nada_cdr(args)), env);
    if (content_val->type != NADA_STRING) {
        fprintf(stderr, "Error: write-file requires a string as second argument\n");
        nada_free(path_val);
        nada_free(content_val);
        return nada_create_bool(0);
    }

    FILE *file = fopen(path_val->data.string, "wb");
    if (!file) {
        fprintf(stderr, "Error: Could not open file %s for writing\n", path_val->data.string);
        nada_free(path_val);
        nada_free(content_val);
        return nada_create_bool(0);
    }

    size_t len = strlen(content_val->data.string);
    size_t written = fwrite(content_val->data.string, 1, len, file);

    fclose(file);

    nada_free(path_val);
    nada_free(content_val);

    return nada_create_bool(written == len);
}

// display: Output a string to console
NadaValue *builtin_display(NadaValue *args, NadaEnv *env) {
    if (nada_is_nil(args)) {
        fprintf(stderr, "Error: display requires at least 1 argument\n");
        return nada_create_nil();
    }

    NadaValue *curr = args;
    while (!nada_is_nil(curr)) {
        NadaValue *val = nada_eval(nada_car(curr), env);

        if (val->type == NADA_STRING) {
            printf("%s", val->data.string);  // No quotes
        } else {
            nada_print(val);  // Use regular printer for non-strings
        }

        nada_free(val);
        curr = nada_cdr(curr);
    }

    printf("\n");  // Add newline at the end
    return nada_create_nil();
}
