#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "NadaString.h"
#include "NadaValue.h"
#include "NadaEval.h"
#include "NadaParser.h"
#include "NadaError.h"
#include "NadaOutput.h"

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

// Helper function to ensure buffer has enough space
static void ensure_buffer_space(char **buffer, int *buffer_size, int pos, int needed) {
    if (pos + needed >= *buffer_size) {
        *buffer_size *= 2;
        *buffer = realloc(*buffer, *buffer_size);
        if (!*buffer) {
            fprintf(stderr, "Error: Out of memory in nada_value_to_string\n");
            exit(1);
        }
    }
}

// Helper function to append to buffer
static void append_to_buffer(char **buffer, int *buffer_size, int *pos, const char *str) {
    int len = strlen(str);
    ensure_buffer_space(buffer, buffer_size, *pos, len + 1);
    strcpy(*buffer + *pos, str);
    *pos += len;
}

// Convert a NadaValue to a string representation
// This is similar to nada_print but writes to a string instead of stdout
char *nada_value_to_string(NadaValue *val) {
    if (val == NULL) {
        return strdup("NULL");
    }

    // Initial buffer size
    int buffer_size = 1024;
    char *buffer = malloc(buffer_size);
    if (!buffer) {
        return NULL;
    }
    buffer[0] = '\0';  // Ensure buffer is initially empty
    int pos = 0;       // Current position in buffer

    switch (val->type) {
    case NADA_NUM: {
        char *num_str = nada_num_to_string(val->data.number);
        append_to_buffer(&buffer, &buffer_size, &pos, num_str);
        free(num_str);
        break;
    }
    case NADA_STRING: {
        append_to_buffer(&buffer, &buffer_size, &pos, "\"");
        append_to_buffer(&buffer, &buffer_size, &pos, val->data.string);
        append_to_buffer(&buffer, &buffer_size, &pos, "\"");
        break;
    }
    case NADA_SYMBOL:
        append_to_buffer(&buffer, &buffer_size, &pos, val->data.symbol);
        break;
    case NADA_NIL:
        append_to_buffer(&buffer, &buffer_size, &pos, "()");
        break;
    case NADA_PAIR: {
        append_to_buffer(&buffer, &buffer_size, &pos, "(");

        // Convert and append the car
        char *car_str = nada_value_to_string(val->data.pair.car);
        append_to_buffer(&buffer, &buffer_size, &pos, car_str);
        free(car_str);

        // Print rest of the list
        NadaValue *rest = val->data.pair.cdr;
        while (rest->type == NADA_PAIR) {
            append_to_buffer(&buffer, &buffer_size, &pos, " ");

            // Convert and append the next car
            char *next_car_str = nada_value_to_string(rest->data.pair.car);
            append_to_buffer(&buffer, &buffer_size, &pos, next_car_str);
            free(next_car_str);

            rest = rest->data.pair.cdr;
        }

        // Handle improper lists
        if (rest->type != NADA_NIL) {
            append_to_buffer(&buffer, &buffer_size, &pos, " . ");

            // Convert and append the final cdr
            char *cdr_str = nada_value_to_string(rest);
            append_to_buffer(&buffer, &buffer_size, &pos, cdr_str);
            free(cdr_str);
        }

        append_to_buffer(&buffer, &buffer_size, &pos, ")");
        break;
    }
    case NADA_ERROR:
        append_to_buffer(&buffer, &buffer_size, &pos, "Error: ");
        append_to_buffer(&buffer, &buffer_size, &pos, val->data.error);
        break;
    case NADA_FUNC:
        append_to_buffer(&buffer, &buffer_size, &pos, "#<function>");
        break;
    case NADA_BOOL:
        append_to_buffer(&buffer, &buffer_size, &pos, val->data.boolean ? "#t" : "#f");
        break;
    }

    return buffer;
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
    // Check args: (substring str start end)
    if (nada_is_nil(args) || nada_is_nil(nada_cdr(args)) ||
        !nada_is_nil(nada_cdr(nada_cdr(nada_cdr(args))))) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "substring requires exactly 3 arguments");
        return nada_create_nil();
    }

    // Evaluate string argument
    NadaValue *str_val = nada_eval(nada_car(args), env);
    if (str_val->type != NADA_STRING) {
        nada_report_error(NADA_ERROR_TYPE_ERROR, "substring requires a string as first argument");
        nada_free(str_val);
        return nada_create_nil();
    }

    // Evaluate start index
    NadaValue *start_val = nada_eval(nada_car(nada_cdr(args)), env);
    if (start_val->type != NADA_NUM) {
        nada_report_error(NADA_ERROR_TYPE_ERROR, "substring requires a number as second argument");
        nada_free(str_val);
        nada_free(start_val);
        return nada_create_nil();
    }

    // Check if start is an integer
    if (!nada_num_is_integer(start_val->data.number)) {
        nada_report_error(NADA_ERROR_TYPE_ERROR, "substring start index must be an integer");
        nada_free(str_val);
        nada_free(start_val);
        return nada_create_nil();
    }

    int start = nada_num_to_int(start_val->data.number);
    if (start < 0) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "substring start index must be non-negative");
        nada_free(str_val);
        nada_free(start_val);
        return nada_create_nil();
    }

    // Evaluate end index
    NadaValue *end_val = nada_eval(nada_car(nada_cdr(nada_cdr(args))), env);
    if (end_val->type != NADA_NUM) {
        nada_report_error(NADA_ERROR_TYPE_ERROR, "substring requires a number as third argument");
        nada_free(str_val);
        nada_free(start_val);
        nada_free(end_val);
        return nada_create_nil();
    }

    // Check if end is an integer
    if (!nada_num_is_integer(end_val->data.number)) {
        nada_report_error(NADA_ERROR_TYPE_ERROR, "substring end index must be an integer");
        nada_free(str_val);
        nada_free(start_val);
        nada_free(end_val);
        return nada_create_nil();
    }

    int end = nada_num_to_int(end_val->data.number);
    if (end < 0) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "substring end index must be non-negative");
        nada_free(str_val);
        nada_free(start_val);
        nada_free(end_val);
        return nada_create_nil();
    }

    if (end < start) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "substring end index must be >= start index");
        nada_free(str_val);
        nada_free(start_val);
        nada_free(end_val);
        return nada_create_nil();
    }

    // Get string length in characters
    int str_len = utf8_strlen(str_val->data.string);

    // Clamp indices to string length
    if (start > str_len) start = str_len;
    if (end > str_len) end = str_len;

    // Get pointers to the start and end positions
    const char *str = str_val->data.string;
    const char *start_ptr = utf8_index(str, start);
    const char *end_ptr = utf8_index(str, end);

    // Calculate byte length and copy the substring
    int byte_len = end_ptr - start_ptr;
    char *result = malloc(byte_len + 1);
    strncpy(result, start_ptr, byte_len);
    result[byte_len] = '\0';

    // Clean up and return
    nada_free(str_val);
    nada_free(start_val);
    nada_free(end_val);

    NadaValue *ret = nada_create_string(result);
    free(result);
    return ret;
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

        // Add the last segment (always add it, even if empty)
        NadaValue *seg_val = nada_create_string(start);
        NadaValue *new_result = nada_cons(seg_val, result);

        // Free temporary values
        nada_free(seg_val);
        nada_free(result);

        // Update result pointer
        result = new_result;

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

// float: Convert number to floating-point string with precision
NadaValue *builtin_float(NadaValue *args, NadaEnv *env) {
    if (nada_is_nil(args) || nada_is_nil(nada_cdr(args)) ||
        !nada_is_nil(nada_cdr(nada_cdr(args)))) {
        fprintf(stderr, "Error: float requires exactly 2 arguments\n");
        return nada_create_nil();
    }

    // Evaluate number argument
    NadaValue *num_val = nada_eval(nada_car(args), env);
    if (num_val->type != NADA_NUM) {
        fprintf(stderr, "Error: float requires a number as first argument\n");
        nada_free(num_val);
        return nada_create_nil();
    }

    // Evaluate precision argument
    NadaValue *prec_val = nada_eval(nada_car(nada_cdr(args)), env);
    if (prec_val->type != NADA_NUM || !nada_num_is_integer(prec_val->data.number)) {
        fprintf(stderr, "Error: float requires an integer precision as second argument\n");
        nada_free(num_val);
        nada_free(prec_val);
        return nada_create_nil();
    }

    int precision = nada_num_to_int(prec_val->data.number);
    if (precision < 0) {
        fprintf(stderr, "Error: float precision must be non-negative\n");
        nada_free(num_val);
        nada_free(prec_val);
        return nada_create_nil();
    }

    char *str = nada_num_to_float_string(num_val->data.number, precision);
    NadaValue *result = nada_create_string(str);

    free(str);
    nada_free(num_val);
    nada_free(prec_val);
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
        nada_write_string("Error: write-to-string requires exactly 1 argument\n");
        return nada_create_error("write-to-string requires exactly 1 argument");
    }

    NadaValue *expr = nada_eval(nada_car(args), env);

    // Use the new abstraction to convert to string
    char *str = nada_value_to_string(expr);
    if (!str) {
        nada_free(expr);
        return nada_create_error("Failed to convert value to string");
    }

    NadaValue *result = nada_create_string(str);
    free(str);
    nada_free(expr);

    return result;
}

// Map of lowercase to uppercase for UTF-8 encoded Unicode characters
typedef struct {
    const char *lower;
    const char *upper;
} CasePair;

static const CasePair unicode_case_map[] = {
    // Latin-1 Supplement (U+00A0 to U+00FF)
    {"à", "À"},
    {"á", "Á"},
    {"â", "Â"},
    {"ã", "Ã"},
    {"ä", "Ä"},
    {"å", "Å"},
    {"æ", "Æ"},
    {"ç", "Ç"},
    {"è", "È"},
    {"é", "É"},
    {"ê", "Ê"},
    {"ë", "Ë"},
    {"ì", "Ì"},
    {"í", "Í"},
    {"î", "Î"},
    {"ï", "Ï"},
    {"ð", "Ð"},
    {"ñ", "Ñ"},
    {"ò", "Ò"},
    {"ó", "Ó"},
    {"ô", "Ô"},
    {"õ", "Õ"},
    {"ö", "Ö"},
    {"ø", "Ø"},
    {"ù", "Ù"},
    {"ú", "Ú"},
    {"û", "Û"},
    {"ü", "Ü"},
    {"ý", "Ý"},
    {"þ", "Þ"},
    {"ÿ", "Ÿ"},

    // Latin Extended-A (U+0100 to U+017F)
    {"ā", "Ā"},
    {"ă", "Ă"},
    {"ą", "Ą"},
    {"ć", "Ć"},
    {"ĉ", "Ĉ"},
    {"ċ", "Ċ"},
    {"č", "Č"},
    {"ď", "Ď"},
    {"đ", "Đ"},
    {"ē", "Ē"},
    {"ĕ", "Ĕ"},
    {"ė", "Ė"},
    {"ę", "Ę"},
    {"ě", "Ě"},
    {"ĝ", "Ĝ"},
    {"ğ", "Ğ"},
    {"ġ", "Ġ"},
    {"ģ", "Ģ"},
    {"ĥ", "Ĥ"},
    {"ħ", "Ħ"},
    {"ĩ", "Ĩ"},
    {"ī", "Ī"},
    {"ĭ", "Ĭ"},
    {"į", "Į"},
    {"ı", "I"},
    {"ĳ", "Ĳ"},
    {"ĵ", "Ĵ"},
    {"ķ", "Ķ"},
    {"ĸ", "K"},
    {"ĺ", "Ĺ"},
    {"ļ", "Ļ"},
    {"ľ", "Ľ"},
    {"ŀ", "Ŀ"},
    {"ł", "Ł"},
    {"ń", "Ń"},
    {"ņ", "Ņ"},
    {"ň", "Ň"},
    {"ŉ", "ʼN"},
    {"ŋ", "Ŋ"},
    {"ō", "Ō"},
    {"ŏ", "Ŏ"},
    {"ő", "Ő"},
    {"œ", "Œ"},
    {"ŕ", "Ŕ"},
    {"ŗ", "Ŗ"},
    {"ř", "Ř"},
    {"ś", "Ś"},
    {"ŝ", "Ŝ"},
    {"ş", "Ş"},
    {"š", "Š"},
    {"ţ", "Ţ"},
    {"ť", "Ť"},
    {"ŧ", "Ŧ"},
    {"ũ", "Ũ"},
    {"ū", "Ū"},
    {"ŭ", "Ŭ"},
    {"ů", "Ů"},
    {"ű", "Ű"},
    {"ų", "Ų"},
    {"ŵ", "Ŵ"},
    {"ŷ", "Ŷ"},
    {"ź", "Ź"},
    {"ż", "Ż"},
    {"ž", "Ž"},

    // Add more mappings as needed...

    {NULL, NULL}  // End marker
};

// Helper function to find a character in the Unicode case map
static const char *find_in_case_map(const char *utf8_char, int to_upper) {
    for (int i = 0; unicode_case_map[i].lower != NULL; i++) {
        if (to_upper) {
            if (strcmp(utf8_char, unicode_case_map[i].lower) == 0) {
                return unicode_case_map[i].upper;
            }
        } else {
            if (strcmp(utf8_char, unicode_case_map[i].upper) == 0) {
                return unicode_case_map[i].lower;
            }
        }
    }
    return NULL;  // Not found
}
// string-upcase: Convert a string to uppercase
NadaValue *builtin_string_upcase(NadaValue *args, NadaEnv *env) {
    if (nada_is_nil(args) || !nada_is_nil(nada_cdr(args))) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "string-upcase requires exactly 1 argument");
        return nada_create_nil();
    }

    NadaValue *str_val = nada_eval(nada_car(args), env);
    if (str_val->type != NADA_STRING) {
        nada_report_error(NADA_ERROR_TYPE_ERROR, "string-upcase requires a string argument");
        nada_free(str_val);
        return nada_create_nil();
    }

    const char *input = str_val->data.string;
    // Allocate buffer (worst case: each UTF-8 char becomes 4 bytes)
    int input_len = strlen(input);
    int buffer_size = input_len * 4 + 1;
    char *result = malloc(buffer_size);
    int pos = 0;

    const char *p = input;
    while (*p) {
        if ((*p & 0x80) == 0) {
            // ASCII character
            result[pos++] = toupper((unsigned char)*p);
            p++;
        } else {
            // UTF-8 character
            int charlen = utf8_charlen(p);
            char utf8_char[5] = {0};
            strncpy(utf8_char, p, charlen);

            // Check if we have a mapping for this character
            const char *mapped = find_in_case_map(utf8_char, 1);  // 1 means to_upper
            if (mapped) {
                int mapped_len = strlen(mapped);
                if (pos + mapped_len < buffer_size) {
                    strcpy(result + pos, mapped);
                    pos += mapped_len;
                }
            } else {
                // No mapping, copy as is
                strncpy(result + pos, p, charlen);
                pos += charlen;
            }
            p += charlen;
        }
    }
    result[pos] = '\0';

    NadaValue *ret = nada_create_string(result);
    free(result);
    nada_free(str_val);

    return ret;
}

// string-downcase: Convert a string to lowercase
NadaValue *builtin_string_downcase(NadaValue *args, NadaEnv *env) {
    if (nada_is_nil(args) || !nada_is_nil(nada_cdr(args))) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "string-downcase requires exactly 1 argument");
        return nada_create_nil();
    }

    NadaValue *str_val = nada_eval(nada_car(args), env);
    if (str_val->type != NADA_STRING) {
        nada_report_error(NADA_ERROR_TYPE_ERROR, "string-downcase requires a string argument");
        nada_free(str_val);
        return nada_create_nil();
    }

    const char *input = str_val->data.string;
    // Allocate buffer (worst case: each UTF-8 char becomes 4 bytes)
    int input_len = strlen(input);
    int buffer_size = input_len * 4 + 1;
    char *result = malloc(buffer_size);
    int pos = 0;

    const char *p = input;
    while (*p) {
        if ((*p & 0x80) == 0) {
            // ASCII character
            result[pos++] = tolower((unsigned char)*p);
            p++;
        } else {
            // UTF-8 character
            int charlen = utf8_charlen(p);
            char utf8_char[5] = {0};
            strncpy(utf8_char, p, charlen);

            // Check if we have a mapping for this character
            const char *mapped = find_in_case_map(utf8_char, 0);  // 0 means to_lower
            if (mapped) {
                int mapped_len = strlen(mapped);
                if (pos + mapped_len < buffer_size) {
                    strcpy(result + pos, mapped);
                    pos += mapped_len;
                }
            } else {
                // No mapping, copy as is
                strncpy(result + pos, p, charlen);
                pos += charlen;
            }
            p += charlen;
        }
    }
    result[pos] = '\0';

    NadaValue *ret = nada_create_string(result);
    free(result);
    nada_free(str_val);

    return ret;
}
