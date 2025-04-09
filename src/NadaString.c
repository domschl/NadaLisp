#include "NadaString.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>  // Add this for isdigit, isalpha, isspace
#include "NadaValue.h"
#include "NadaEval.h"
#include "NadaParser.h"
#include "NadaError.h"

// Calculate the number of UTF-8 characters in a string
int utf8_strlen(const char *str) {
    int count = 0;
    while (*str) {
        if ((*str & 0xC0) != 0x80) {  // Not a continuation byte
            count++;
        }
        str++;
    }
    return count;
}

// Get pointer to the nth UTF-8 character
const char *utf8_index(const char *str, int index) {
    int count = 0;
    const char *p = str;

    while (*p && count < index) {
        if ((*p & 0xC0) != 0x80) {  // Not a continuation byte
            count++;
        }
        p++;
    }

    return p;
}

// Get the byte length of a UTF-8 character
int utf8_charlen(const char *str) {
    unsigned char c = (unsigned char)*str;

    if (c < 0x80) return 1;
    if (c < 0xE0) return 2;
    if (c < 0xF0) return 3;
    return 4;
}

// string-length: Get length of a string in characters
NadaValue *builtin_string_length(NadaValue *args, NadaEnv *env) {
    if (nada_is_nil(args) || !nada_is_nil(nada_cdr(args))) {
        fprintf(stderr, "Error: string-length requires exactly 1 argument\n");
        return nada_create_nil();
    }

    NadaValue *str_val = nada_eval(nada_car(args), env);
    if (str_val->type != NADA_STRING) {
        fprintf(stderr, "Error: string-length requires a string argument\n");
        nada_free(str_val);
        return nada_create_nil();
    }

    int length = utf8_strlen(str_val->data.string);
    nada_free(str_val);

    return nada_create_num_from_int(length);
}

// substring: Extract a substring
NadaValue *builtin_substring(NadaValue *args, NadaEnv *env) {
    // Check args: (substring str start [length])
    if (nada_is_nil(args) || nada_is_nil(nada_cdr(args)) ||
        !nada_is_nil(nada_cdr(nada_cdr(nada_cdr(args))))) {
        fprintf(stderr, "Error: substring requires 2 or 3 arguments\n");
        return nada_create_nil();
    }

    // Evaluate string argument
    NadaValue *str_val = nada_eval(nada_car(args), env);
    if (str_val->type != NADA_STRING) {
        fprintf(stderr, "Error: substring requires a string as first argument\n");
        nada_free(str_val);
        return nada_create_nil();
    }

    // Evaluate start index
    NadaValue *start_val = nada_eval(nada_car(nada_cdr(args)), env);
    if (start_val->type != NADA_NUM) {
        fprintf(stderr, "Error: substring requires a number as second argument\n");
        nada_free(str_val);
        nada_free(start_val);
        return nada_create_nil();
    }

    // Check if start is an integer
    if (!nada_num_is_integer(start_val->data.number)) {
        fprintf(stderr, "Error: substring start index must be an integer\n");
        nada_free(str_val);
        nada_free(start_val);
        return nada_create_nil();
    }

    int start = nada_num_to_int(start_val->data.number);
    if (start < 0) {
        fprintf(stderr, "Error: substring start index must be non-negative\n");
        nada_free(str_val);
        nada_free(start_val);
        return nada_create_nil();
    }

    // Optional length parameter
    int length = -1;  // Default to rest of string
    if (!nada_is_nil(nada_cdr(nada_cdr(args)))) {
        NadaValue *length_val = nada_eval(nada_car(nada_cdr(nada_cdr(args))), env);
        if (length_val->type != NADA_NUM) {
            fprintf(stderr, "Error: substring requires a number as third argument\n");
            nada_free(str_val);
            nada_free(start_val);
            nada_free(length_val);
            return nada_create_nil();
        }

        // Check if length is an integer
        if (!nada_num_is_integer(length_val->data.number)) {
            fprintf(stderr, "Error: substring length must be an integer\n");
            nada_free(str_val);
            nada_free(start_val);
            nada_free(length_val);
            return nada_create_nil();
        }

        length = nada_num_to_int(length_val->data.number);
        if (length < 0) {
            fprintf(stderr, "Error: substring length must be non-negative\n");
            nada_free(str_val);
            nada_free(start_val);
            nada_free(length_val);
            return nada_create_nil();
        }

        nada_free(length_val);
    }

    // The rest of the function remains unchanged
    // Get pointers to the relevant positions
    const char *str = str_val->data.string;
    const char *start_ptr = utf8_index(str, start);

    if (*start_ptr == '\0') {
        // Start index is beyond the string
        nada_free(str_val);
        nada_free(start_val);
        return nada_create_string("");
    }

    if (length < 0) {
        // Extract to the end of the string
        nada_free(start_val);
        char *result = strdup(start_ptr);
        nada_free(str_val);
        return nada_create_string(result);
    } else {
        // Extract specific number of characters
        const char *end_ptr = start_ptr;
        for (int i = 0; i < length && *end_ptr; i++) {
            end_ptr = utf8_index(end_ptr, 1);
        }

        int byte_len = end_ptr - start_ptr;
        char *result = malloc(byte_len + 1);
        strncpy(result, start_ptr, byte_len);
        result[byte_len] = '\0';

        nada_free(str_val);
        nada_free(start_val);

        NadaValue *ret = nada_create_string(result);
        free(result);
        return ret;
    }
}

// string-split: Split a string by delimiter or into characters
NadaValue *builtin_string_split(NadaValue *args, NadaEnv *env) {
    // Check args: (string-split str [delimiter])
    if (nada_is_nil(args) || !nada_is_nil(nada_cdr(nada_cdr(args)))) {
        fprintf(stderr, "Error: string-split requires 1 or 2 arguments\n");
        return nada_create_nil();
    }

    // Evaluate string argument
    NadaValue *str_val = nada_eval(nada_car(args), env);
    if (str_val->type != NADA_STRING) {
        fprintf(stderr, "Error: string-split requires a string as first argument\n");
        nada_free(str_val);
        return nada_create_nil();
    }

    const char *str = str_val->data.string;

    // Check if delimiter is provided
    if (nada_is_nil(nada_cdr(args))) {
        // Split into individual characters
        NadaValue *result = nada_create_nil();
        const char *p = str;

        while (*p) {
            int charlen = utf8_charlen(p);
            char *ch = malloc(charlen + 1);
            strncpy(ch, p, charlen);
            ch[charlen] = '\0';

            // Create string value
            NadaValue *ch_val = nada_create_string(ch);
            
            // Build list in reverse order
            NadaValue *new_result = nada_cons(ch_val, result);
            
            // Free temporary values
            nada_free(ch_val);
            nada_free(result);
            
            // Update result pointer
            result = new_result;
            
            free(ch);
            p += charlen;
        }

        // Reverse the list
        NadaValue *reversed = nada_reverse(result);
        
        // Free the intermediate list
        nada_free(result);
        nada_free(str_val);
        
        return reversed;
    } else {
        // Split by delimiter
        NadaValue *delim_val = nada_eval(nada_car(nada_cdr(args)), env);
        if (delim_val->type != NADA_STRING) {
            fprintf(stderr, "Error: string-split requires a string as second argument\n");
            nada_free(str_val);
            nada_free(delim_val);
            return nada_create_nil();
        }

        const char *delim = delim_val->data.string;
        int delim_len = strlen(delim);

        if (delim_len == 0) {
            // Empty delimiter, return the original string as a single element
            NadaValue *str_copy = nada_create_string(str);
            NadaValue *nil_val = nada_create_nil();
            NadaValue *result = nada_cons(str_copy, nil_val);
            
            // Free temporary values
            nada_free(str_copy);
            nada_free(nil_val);
            nada_free(str_val);
            nada_free(delim_val);
            
            return result;
        }

        // Split by delimiter
        NadaValue *result = nada_create_nil();
        const char *start = str;
        const char *found;

        while ((found = strstr(start, delim)) != NULL) {
            int len = found - start;
            char *segment = malloc(len + 1);
            strncpy(segment, start, len);
            segment[len] = '\0';

            // Create string value
            NadaValue *seg_val = nada_create_string(segment);
            
            // Add to result list
            NadaValue *new_result = nada_cons(seg_val, result);
            
            // Free temporary values
            nada_free(seg_val);
            nada_free(result);
            
            // Update result pointer
            result = new_result;
            
            free(segment);
            start = found + delim_len;
        }

        // Add the last segment
        if (*start) {
            NadaValue *seg_val = nada_create_string(start);
            NadaValue *new_result = nada_cons(seg_val, result);
            
            // Free temporary values
            nada_free(seg_val);
            nada_free(result);
            
            // Update result pointer
            result = new_result;
        }

        // Reverse the list
        NadaValue *reversed = nada_reverse(result);
        
        // Free intermediate values
        nada_free(result);
        nada_free(str_val);
        nada_free(delim_val);
        
        return reversed;
    }
}

// string-join: Join a list of strings
NadaValue *builtin_string_join(NadaValue *args, NadaEnv *env) {
    // Check args: (string-join list [delimiter])
    if (nada_is_nil(args)) {
        fprintf(stderr, "Error: string-join requires at least 1 argument\n");
        return nada_create_nil();
    }

    // Evaluate list argument
    NadaValue *list_val = nada_eval(nada_car(args), env);
    if (list_val->type != NADA_PAIR && list_val->type != NADA_NIL) {
        fprintf(stderr, "Error: string-join requires a list as first argument\n");
        nada_free(list_val);
        return nada_create_nil();
    }

    // Evaluate delimiter if provided
    const char *delim = "";
    NadaValue *delim_val = NULL;

    // Check if second argument exists before accessing it
    if (!nada_is_nil(args) && !nada_is_nil(nada_cdr(args))) {
        delim_val = nada_eval(nada_car(nada_cdr(args)), env);
        if (delim_val->type != NADA_STRING) {
            fprintf(stderr, "Error: string-join requires a string as second argument\n");
            nada_free(list_val);
            nada_free(delim_val);
            return nada_create_nil();
        }
        delim = delim_val->data.string;
    }

    // Calculate total length
    int total_len = 0;
    int delim_len = strlen(delim);
    int count = 0;
    NadaValue *curr = list_val;

    while (!nada_is_nil(curr)) {
        NadaValue *item = nada_car(curr);
        if (item->type != NADA_STRING) {
            fprintf(stderr, "Error: string-join requires all list elements to be strings\n");
            nada_free(list_val);
            if (delim_val) nada_free(delim_val);
            return nada_create_nil();
        }

        total_len += strlen(item->data.string);
        count++;
        curr = nada_cdr(curr);
    }

    if (count > 0) {
        total_len += delim_len * (count - 1);
    }

    // Allocate and build the result
    char *result = malloc(total_len + 1);
    result[0] = '\0';

    curr = list_val;
    int first = 1;

    while (!nada_is_nil(curr)) {
        if (!first) {
            strcat(result, delim);
        }

        strcat(result, nada_car(curr)->data.string);
        first = 0;
        curr = nada_cdr(curr);
    }

    nada_free(list_val);
    if (delim_val) nada_free(delim_val);

    NadaValue *ret = nada_create_string(result);
    free(result);
    return ret;
}

// string->number: Convert string to number
NadaValue *builtin_string_to_number(NadaValue *args, NadaEnv *env) {
    if (nada_is_nil(args) || !nada_is_nil(nada_cdr(args))) {
        fprintf(stderr, "Error: string->number requires exactly 1 argument\n");
        return nada_create_nil();
    }

    NadaValue *str_val = nada_eval(nada_car(args), env);
    if (str_val->type != NADA_STRING) {
        fprintf(stderr, "Error: string->number requires a string argument\n");
        nada_free(str_val);
        return nada_create_nil();
    }

    if (!nada_is_valid_number_string(str_val->data.string)) {
        nada_free(str_val);
        return nada_create_bool(0);  // Return #f for invalid number
    }

    NadaValue *result = nada_create_num_from_string(str_val->data.string);
    nada_free(str_val);
    return result;
}

// number->string: Convert number to string
NadaValue *builtin_number_to_string(NadaValue *args, NadaEnv *env) {
    if (nada_is_nil(args) || !nada_is_nil(nada_cdr(args))) {
        fprintf(stderr, "Error: number->string requires exactly 1 argument\n");
        return nada_create_nil();
    }

    NadaValue *num_val = nada_eval(nada_car(args), env);
    if (num_val->type != NADA_NUM) {
        fprintf(stderr, "Error: number->string requires a number argument\n");
        nada_free(num_val);
        return nada_create_nil();
    }

    char *str = nada_num_to_string(num_val->data.number);
    NadaValue *result = nada_create_string(str);

    free(str);
    nada_free(num_val);
    return result;
}

// string->symbol: Convert string to symbol
NadaValue *builtin_string_to_symbol(NadaValue *args, NadaEnv *env) {
    if (nada_is_nil(args) || !nada_is_nil(nada_cdr(args))) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT,
                          "string->symbol requires exactly one string argument");
        return nada_create_nil();
    }

    NadaValue *str_arg = nada_eval(nada_car(args), env);
    if (str_arg->type != NADA_STRING) {
        nada_report_error(NADA_ERROR_TYPE_ERROR,
                          "string->symbol requires a string argument");
        nada_free(str_arg);
        return nada_create_nil();
    }

    // Create a symbol from the string
    NadaValue *result = nada_create_symbol(str_arg->data.string);
    nada_free(str_arg);
    return result;
}

// Updated tokenize-expr function
NadaValue *builtin_tokenize_expr(NadaValue *args, NadaEnv *env) {
    // Check args
    if (nada_is_nil(args) || !nada_is_nil(nada_cdr(args))) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT,
                          "tokenize-expr requires exactly one string argument");
        return nada_create_nil();
    }

    // Evaluate the argument
    NadaValue *str_arg = nada_eval(nada_car(args), env);
    if (str_arg->type != NADA_STRING) {
        nada_report_error(NADA_ERROR_TYPE_ERROR,
                          "tokenize-expr requires a string argument");
        nada_free(str_arg);
        return nada_create_nil();
    }

    // Implement tokenization logic here
    // Split the string into tokens (numbers, operators, parentheses)
    // Return a list of string tokens

    // Example implementation outline:
    const char *input = str_arg->data.string;
    NadaValue *tokens = nada_create_nil();

    // Simple tokenizer for algebraic expressions
    char token_buf[256];
    int token_pos = 0;

    for (int i = 0; input[i] != '\0'; i++) {
        char c = input[i];

        // Handle operators and parentheses as single-character tokens
        if (c == '+' || c == '-' || c == '*' || c == '/' ||
            c == '^' || c == '(' || c == ')') {

            // If we have a pending token, add it first
            if (token_pos > 0) {
                token_buf[token_pos] = '\0';
                NadaValue *token = nada_create_string(token_buf);
                NadaValue *new_tokens = nada_cons(token, tokens); // Create new list with token
                nada_free(token);  // Free the token after it's been copied
                nada_free(tokens); // Free the old list
                tokens = new_tokens; // Update our list pointer
                token_pos = 0;
            }

            // Add the operator token
            token_buf[0] = c;
            token_buf[1] = '\0';
            NadaValue *op_token = nada_create_string(token_buf);
            NadaValue *new_tokens = nada_cons(op_token, tokens); // Create new list with op_token
            nada_free(op_token); // Free the token after it's been copied
            nada_free(tokens);   // Free the old list
            tokens = new_tokens; // Update our list pointer

        } else if (isdigit(c) || isalpha(c) || c == '.') {
            // Build number or variable tokens
            token_buf[token_pos++] = c;
        } else if (isspace(c)) {
            // Finish current token if any
            if (token_pos > 0) {
                token_buf[token_pos] = '\0';
                NadaValue *token = nada_create_string(token_buf);
                NadaValue *new_tokens = nada_cons(token, tokens); // Create new list with token
                nada_free(token);  // Free the token after it's been copied
                nada_free(tokens); // Free the old list 
                tokens = new_tokens; // Update our list pointer
                token_pos = 0;
            }
        }
    }

    // Add any final token
    if (token_pos > 0) {
        token_buf[token_pos] = '\0';
        NadaValue *token = nada_create_string(token_buf);
        NadaValue *new_tokens = nada_cons(token, tokens); // Create new list with token
        nada_free(token);  // Free the token after it's been copied
        nada_free(tokens); // Free the old list
        tokens = new_tokens; // Update our list pointer
    }

    // Reverse the tokens list to get them in the original order
    NadaValue *result = nada_reverse(tokens);
    nada_free(tokens); // Free the intermediate list
    nada_free(str_arg); // Free the evaluated input string
    
    return result;
}

// Built-in function: read-from-string
NadaValue *builtin_read_from_string(NadaValue *args, NadaEnv *env) {
    if (nada_is_nil(args) || !nada_is_nil(nada_cdr(args))) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "read-from-string requires exactly one string argument");
        return nada_create_nil();
    }

    // Evaluate the argument
    NadaValue *str_arg = nada_eval(nada_car(args), env);
    if (str_arg->type != NADA_STRING) {
        nada_report_error(NADA_ERROR_TYPE_ERROR, "read-from-string requires a string argument");
        nada_free(str_arg);
        return nada_create_nil();
    }

    // Parse the string content
    NadaValue *parsed = nada_parse(str_arg->data.string);
    
    // Free the evaluated argument
    nada_free(str_arg);
    
    // If parsing failed, return nil
    if (!parsed) {
        nada_report_error(NADA_ERROR_SYNTAX, "failed to parse string");
        return nada_create_nil();
    }
    
    return parsed;
}

// write-to-string: Convert a Lisp expression to a string
NadaValue *builtin_write_to_string(NadaValue *args, NadaEnv *env) {
    if (nada_is_nil(args) || !nada_is_nil(nada_cdr(args))) {
        fprintf(stderr, "Error: write-to-string requires exactly 1 argument\n");
        return nada_create_nil();
    }

    NadaValue *expr = nada_eval(nada_car(args), env);

    // Write to a string buffer
    char buffer[4096];  // Fixed size for simplicity
    FILE *memstream = fmemopen(buffer, sizeof(buffer), "w");

    if (!memstream) {
        fprintf(stderr, "Error: Failed to create memory stream\n");
        nada_free(expr);
        return nada_create_nil();
    }

    FILE *old_stdout = stdout;
    stdout = memstream;

    nada_print(expr);

    stdout = old_stdout;
    fclose(memstream);

    nada_free(expr);
    return nada_create_string(buffer);
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

// read-line: Read a line from console
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

// eval: Evaluate a quoted expression
NadaValue *builtin_eval(NadaValue *args, NadaEnv *env) {
    if (nada_is_nil(args) || !nada_is_nil(nada_cdr(args))) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "eval requires exactly one argument");
        return nada_create_nil();
    }

    // First evaluate the argument (to get the expression to evaluate)
    NadaValue *expr = nada_eval(nada_car(args), env);
    
    // Then evaluate the resulting expression
    NadaValue *result = nada_eval(expr, env);
    
    // Free the intermediate expression value
    nada_free(expr);
    
    return result;
}
