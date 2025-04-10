#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "NadaEval.h"
#include "NadaParser.h"
#include "NadaString.h"
#include "NadaError.h"
#include "NadaBuiltinLists.h"
#include "NadaBuiltinMath.h"
#include "NadaBuiltinCompare.h"
#include "NadaBuiltinSpecialForms.h"
#include "NadaBuiltinPredicates.h"
#include "NadaBuiltinBoolOps.h"


// Forward declaration of the builtins array
static BuiltinFuncInfo builtins[];

// Flag to control symbol error reporting
static int g_silent_symbol_lookup = 0;

// Function to enable/disable silent symbol lookup
void nada_set_silent_symbol_lookup(int silent) {
    g_silent_symbol_lookup = silent;
}

// Apply a function to arguments
NadaValue *apply_function(NadaValue *func, NadaValue *args, NadaEnv *outer_env) {
    if (func->type != NADA_FUNC) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "attempt to apply non-function");
        return nada_create_nil();
    }

    // Handle built-in functions
    if (func->data.function.builtin != NULL) {
        // Create a list to hold evaluated arguments
        NadaValue *eval_args = nada_create_nil();
        NadaValue *current_arg = args;

        // Evaluate each argument
        while (!nada_is_nil(current_arg)) {
            // Evaluate the current argument
            NadaValue *arg_val = nada_eval(nada_car(current_arg), outer_env);

            // Prepend to our list (we'll reverse it later)
            NadaValue *new_eval_args = nada_cons(arg_val, eval_args);
            nada_free(arg_val);
            nada_free(eval_args);
            eval_args = new_eval_args;

            // Move to next argument
            current_arg = nada_cdr(current_arg);
        }

        // Reverse the list to get arguments in the correct order
        NadaValue *reversed_args = nada_reverse(eval_args);

        // Free the intermediate list
        nada_free(eval_args);

        // Call the built-in function with evaluated arguments
        NadaValue *result = func->data.function.builtin(reversed_args, outer_env);

        // Free our intermediate lists
        nada_free(reversed_args);

        return result;
    }

    // For user-defined functions:

    // First, create a new environment with the closure as parent
    NadaEnv *func_env = nada_env_create(func->data.function.env);

    // Bind arguments to parameters
    NadaValue *param = func->data.function.params;
    NadaValue *arg = args;

    while (!nada_is_nil(param) && !nada_is_nil(arg)) {
        // Get parameter name
        NadaValue *param_name = nada_car(param);

        // Evaluate argument
        NadaValue *arg_val = nada_eval(nada_car(arg), outer_env);

        // Check for evaluation errors
        if (!arg_val) {
            nada_env_release(func_env);  // Clean up on error path
            return nada_create_nil();
        }

        // Bind parameter to argument value
        nada_env_set(func_env, param_name->data.symbol, arg_val);
        nada_free(arg_val);

        // Move to next parameter and argument
        param = nada_cdr(param);
        arg = nada_cdr(arg);
    }

    // Parameter count error checking
    if (!nada_is_nil(param) || !nada_is_nil(arg)) {
        if (!nada_is_nil(param)) {
            nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "too few arguments");
        } else {
            nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "too many arguments");
        }
        nada_env_release(func_env);  // Clean up on error path
        return nada_create_nil();
    }

    // Evaluate the body expressions
    NadaValue *result = nada_create_nil();
    NadaValue *body_expr = func->data.function.body;

    while (!nada_is_nil(body_expr)) {
        // Free previous result before getting new one
        nada_free(result);

        // Evaluate current expression
        result = nada_eval(nada_car(body_expr), func_env);

        // Move to next expression
        body_expr = nada_cdr(body_expr);
    }

    // CRITICAL: Make a deep copy of the result before releasing the environment
    NadaValue *final_result = nada_deep_copy(result);
    nada_free(result);

    // ALWAYS release the function environment
    nada_env_release(func_env);

    return final_result;
}

// Helper to check if a name is a built-in function
static int is_builtin(const char *name) {
    for (int i = 0; builtins[i].name != NULL; i++) {
        if (strcmp(name, builtins[i].name) == 0) {
            return 1;
        }
    }
    return 0;
}

// Built-in function predicate
static NadaValue *builtin_builtin_p(NadaValue *args, NadaEnv *env) {
    if (nada_is_nil(args) || !nada_is_nil(nada_cdr(args))) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "builtin? requires exactly 1 argument");
        return nada_create_bool(0);
    }

    // First get the unevaluated argument
    NadaValue *arg = nada_car(args);

    // We only check symbols
    if (arg->type == NADA_SYMBOL) {
        int result = is_builtin(arg->data.symbol);
        return nada_create_bool(result);
    }

    // For non-symbols, evaluate and then check if the result is a built-in
    NadaValue *val = nada_eval(arg, env);
    int result = 0;

    if (val->type == NADA_SYMBOL) {
        result = is_builtin(val->data.symbol);
    }

    nada_free(val);
    return nada_create_bool(result);
}

// Helper function to collect symbols from an environment
void collect_symbols(NadaEnv *current_env, NadaValue **list) {
    struct NadaBinding *binding = current_env->bindings;
    while (binding != NULL) {
        // Create the symbol value
        NadaValue *symbol = nada_create_symbol(binding->name);

        // Add symbol to the front of our list
        NadaValue *new_list = nada_cons(symbol, *list);

        // Free the temporary symbol and previous list
        nada_free(symbol);
        nada_free(*list);

        // Update the list pointer
        *list = new_list;

        binding = binding->next;
    }

    // If we have a parent environment, collect from there too
    if (current_env->parent != NULL) {
        collect_symbols(current_env->parent, list);
    }
}

// Built-in function: env-symbols
static NadaValue *builtin_env_symbols(NadaValue *args, NadaEnv *env) {
    if (!nada_is_nil(args)) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "env-symbols takes no arguments");
        return nada_create_nil();
    }

    // Collect all symbols in current environment
    NadaValue *result = nada_create_nil();

    collect_symbols(env, &result);
    return result;
}

// Helper function to print bindings
void print_bindings(NadaEnv *current_env, int level) {
    struct NadaBinding *binding = current_env->bindings;
    while (binding != NULL) {
        printf("%*s%s: ", level * 2, "", binding->name);

        // Print type-specific information
        switch (binding->value->type) {
        case NADA_NUM: {
            char *num_str = nada_num_to_string(binding->value->data.number);
            printf("Number (%s)\n", num_str);
            free(num_str);
            break;
        }
        case NADA_STRING:
            printf("String (\"%s\")\n", binding->value->data.string);
            break;
        case NADA_SYMBOL:
            printf("Symbol (%s)\n", binding->value->data.symbol);
            break;
        case NADA_BOOL:
            printf("Boolean (%s)\n", binding->value->data.boolean ? "#t" : "#f");
            break;
        case NADA_PAIR:
            printf("List\n");
            break;
        case NADA_FUNC:
            printf("Function\n");
            break;
        case NADA_NIL:
            printf("Nil\n");
            break;
        }

        binding = binding->next;
    }

    // If we have a parent environment, print its bindings too
    if (current_env->parent != NULL) {
        printf("%*sParent environment:\n", level * 2, "");
        print_bindings(current_env->parent, level + 1);
    }
}

// Built-in function: env-describe
static NadaValue *builtin_env_describe(NadaValue *args, NadaEnv *env) {
    if (!nada_is_nil(args)) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "env-describe takes no arguments");
        return nada_create_nil();
    }

    printf("Environment contents:\n");

    print_bindings(env, 0);

    return nada_create_nil();  // Just return nil
}

// Helper to serialize a value to the file
void serialize_value(NadaValue *val, FILE *f) {
    switch (val->type) {
    case NADA_NUM: {
        char *num_str = nada_num_to_string(val->data.number);
        fprintf(f, "%s", num_str);
        free(num_str);
        break;
    }
    case NADA_STRING:
        fprintf(f, "\"%s\"", val->data.string);
        break;
    case NADA_SYMBOL:
        fprintf(f, "%s", val->data.symbol);
        break;
    case NADA_BOOL:
        fprintf(f, "%s", val->data.boolean ? "#t" : "#f");
        break;
    case NADA_NIL:
        fprintf(f, "()");
        break;
    case NADA_PAIR: {
        fprintf(f, "(");
        NadaValue *current = val;
        int first = 1;
        while (current->type == NADA_PAIR) {
            if (!first) fprintf(f, " ");
            serialize_value(current->data.pair.car, f);
            current = current->data.pair.cdr;
            first = 0;
        }
        if (current->type != NADA_NIL) {
            fprintf(f, " . ");
            serialize_value(current, f);
        }
        fprintf(f, ")");
        break;
    }
    case NADA_FUNC:
        fprintf(f, "#<function>");
        break;
    }
}

// Write all definitions to the file (skipping builtins)
void serialize_env(NadaEnv *current_env, FILE *out) {
    struct NadaBinding *binding = current_env->bindings;
    while (binding != NULL) {
        // Skip built-in functions
        if (!is_builtin(binding->name)) {
            fprintf(out, "(define %s ", binding->name);
            serialize_value(binding->value, out);
            fprintf(out, ")\n");
        }
        binding = binding->next;
    }

    // We don't save parent environments
}

// Built-in function: save-environment
static NadaValue *builtin_save_environment(NadaValue *args, NadaEnv *env) {
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

    serialize_env(env, file);
    fclose(file);

    nada_free(filename_arg);
    return nada_create_bool(1);  // Return true for success
}

// Built-in function: load-file
static NadaValue *builtin_load_file(NadaValue *args, NadaEnv *env) {
    if (nada_is_nil(args) || !nada_is_nil(nada_cdr(args))) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "load-file requires exactly one filename argument");
        return nada_create_bool(0);
    }

    NadaValue *filename_arg = nada_eval(nada_car(args), env);
    if (filename_arg->type != NADA_STRING) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "load-file requires a string filename");
        nada_free(filename_arg);
        return nada_create_bool(0);
    }

    FILE *file = fopen(filename_arg->data.string, "r");
    if (!file) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "could not open file %s for reading",
                          filename_arg->data.string);
        nada_free(filename_arg);
        return nada_create_bool(0);
    }

    // Use dynamic buffer allocation to handle files of any size
    size_t buffer_size = 16384;  // Start with a larger initial buffer
    char *buffer = malloc(buffer_size);
    if (!buffer) {
        nada_report_error(NADA_ERROR_MEMORY, "failed to allocate memory for loading file");
        fclose(file);
        nada_free(filename_arg);
        return nada_create_bool(0);
    }
    buffer[0] = '\0';

    char line[1024];
    int paren_balance = 0;
    int in_string = 0;
    int prev_char_backslash = 0;  // Track backslashes for escaped quotes
    NadaValue *last_result = nada_create_nil();

    while (fgets(line, sizeof(line), file)) {
        // Skip full-line comments
        if (line[0] == ';') continue;

        // Check if we need to expand the buffer
        if (strlen(buffer) + strlen(line) + 1 > buffer_size) {
            buffer_size *= 2;
            char *new_buffer = realloc(buffer, buffer_size);
            if (!new_buffer) {
                nada_report_error(NADA_ERROR_MEMORY, "failed to expand buffer for loading file");
                free(buffer);
                fclose(file);
                nada_free(filename_arg);
                nada_free(last_result);
                return nada_create_bool(0);
            }
            buffer = new_buffer;
        }

        // Add this line to our buffer
        strcat(buffer, line);

        // Count parentheses to ensure we have complete expressions
        for (char *p = line; *p; p++) {
            // Check for comments - comments start with semicolon and continue to end of line
            if (*p == ';' && !in_string) {
                // Found a comment, ignore rest of line
                break;
            }

            // Handle string delimiters correctly (respect escaping)
            if (*p == '"' && !prev_char_backslash) {
                in_string = !in_string;
            }

            // Track backslash for escaped quotes
            prev_char_backslash = (*p == '\\' && !prev_char_backslash);

            // Only count parentheses outside of strings
            if (!in_string) {
                if (*p == '(')
                    paren_balance++;
                else if (*p == ')')
                    paren_balance--;
            }
        }

        // If balanced and buffer isn't empty, evaluate the expression
        if (paren_balance == 0 && strlen(buffer) > 0) {
            NadaValue *expr = nada_parse(buffer);
            if (expr) {
                // Free previous result
                nada_free(last_result);

                // Evaluate this expression
                last_result = nada_eval(expr, env);
                nada_free(expr);
            }

            // Clear buffer for next expression
            buffer[0] = '\0';
        }
    }

    // Process any remaining content in the buffer
    if (strlen(buffer) > 0) {
        if (paren_balance != 0) {
            nada_report_error(NADA_ERROR_SYNTAX, "unbalanced parentheses in file %s",
                              filename_arg->data.string);
        } else {
            NadaValue *expr = nada_parse(buffer);
            if (expr) {
                nada_free(last_result);
                last_result = nada_eval(expr, env);
                nada_free(expr);
            }
        }
    }

    free(buffer);
    fclose(file);
    nada_free(filename_arg);

    return last_result;  // Return the result of the last evaluation
}

// Built-in function: define-test
static NadaValue *builtin_define_test(NadaValue *args, NadaEnv *env) {
    // Ensure we have at least a name and one expression
    if (nada_is_nil(args) || nada_is_nil(nada_cdr(args))) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "define-test requires a name and at least one test expression");
        return nada_create_nil();
    }

    // Extract the test name
    NadaValue *name_val = nada_car(args);
    if (name_val->type != NADA_STRING) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "define-test name must be a string");
        return nada_create_nil();
    }
    char *test_name = name_val->data.string;

    // Get all the body expressions
    NadaValue *body = nada_cdr(args);

    // Begin the test
    printf("Running test: %s\n", test_name);
    int passed = 1;

    // Evaluate each expression in the body
    while (!nada_is_nil(body)) {
        NadaValue *expr = nada_car(body);
        NadaValue *result = nada_eval(expr, env);

        // If any expression returns false, the test fails
        if (result->type == NADA_BOOL && result->data.boolean == 0) {
            passed = 0;
        }

        nada_free(result);
        body = nada_cdr(body);
    }

    // Report test results
    if (passed) {
        printf("Test '%s' PASSED\n", test_name);
    } else {
        printf("Test '%s' FAILED\n", test_name);
    }

    return nada_create_bool(passed);
}

// Add to the builtins table (keep all string functions here)
static BuiltinFuncInfo builtins[] = {
    {"quote", builtin_quote},
    {"car", builtin_car},
    {"cdr", builtin_cdr},
    {"cadr", builtin_cadr},    // Add cadr function
    {"caddr", builtin_caddr},  // Add caddr function
    {"+", builtin_add},
    {"-", builtin_subtract},
    {"*", builtin_multiply},
    {"/", builtin_divide},
    {"%", builtin_modulo},
    {"modulo", builtin_modulo},  // Add modulo as alias
    {"define", builtin_define},
    {"lambda", builtin_lambda},
    {"<", builtin_less_than},
    {"<=", builtin_less_equal},
    {">", builtin_greater_than},
    {">=", builtin_greater_equal},
    {"=", builtin_numeric_equal},
    {"eq?", builtin_eq},
    {"equal?", builtin_equal},
    {"null?", builtin_null},
    {"cond", builtin_cond},
    {"let", builtin_let},
    {"env-symbols", builtin_env_symbols},
    {"env-describe", builtin_env_describe},
    {"save-environment", builtin_save_environment},
    {"load-file", builtin_load_file},
    {"undef", builtin_undef},
    {"integer?", builtin_integer_p},
    {"number?", builtin_number_p},  // New number? predicate
    {"string?", builtin_string_p},
    {"symbol?", builtin_symbol_p},
    {"boolean?", builtin_boolean_p},
    {"pair?", builtin_pair_p},
    {"function?", builtin_function_p},
    {"list?", builtin_list_p},
    {"atom?", builtin_atom_p},
    {"builtin?", builtin_builtin_p},

    // String operations
    {"string-length", builtin_string_length},
    {"substring", builtin_substring},
    {"string-split", builtin_string_split},
    {"string-join", builtin_string_join},
    {"string->number", builtin_string_to_number},
    {"number->string", builtin_number_to_string},
    {"tokenize-expr", builtin_tokenize_expr},  // Add this line
    {"read-from-string", builtin_read_from_string},
    {"write-to-string", builtin_write_to_string},
    {"string->symbol", builtin_string_to_symbol},  // Add this line

    // I/O operations
    {"read-file", builtin_read_file},
    {"write-file", builtin_write_file},
    {"display", builtin_display},
    {"read-line", builtin_read_line},

    // Evaluation
    {"eval", builtin_eval},

    // Cons function: Create a pair
    {"cons", builtin_cons},
    // List function: Create a proper list
    {"list", builtin_list},

    // Add the new if function
    {"if", builtin_if},

    // Add the new length function
    {"length", builtin_length},

    // Add the new define-test function
    {"define-test", builtin_define_test},

    // Add the new begin function
    {"begin", builtin_begin},

    // Add the new or function
    {"or", builtin_or},

    // Add the new and function
    {"and", builtin_and},

    // Add the new sublist function
    {"sublist", builtin_sublist},

    // Add the new list-ref function
    {"list-ref", builtin_list_ref},

    // Add the new not function
    {"not", builtin_not},

    // Add the new map function
    {"map", builtin_map},

    // Add the new set! function
    {"set!", builtin_set},

    {NULL, NULL}  // Sentinel to mark end of array
};

// Create a standard environment with basic operations
NadaEnv *nada_standard_env(void) {
    NadaEnv *env = nada_env_create(NULL);

    // Add built-in functions to environment
    for (int i = 0; builtins[i].name != NULL; i++) {
        // For simplicity, we're not creating proper function objects yet
        // We'll just use symbols with special handling in the evaluator
        nada_env_set(env, builtins[i].name, nada_create_symbol(builtins[i].name));
    }

    return env;
}

// Create a standard environment with all built-in functions
NadaEnv *nada_create_standard_env(void) {
    // Add tracking here
    NadaEnv *env = nada_env_create(NULL);

    // Register all built-in functions from the builtins array
    for (int i = 0; builtins[i].name != NULL; i++) {
        NadaValue *func = nada_create_builtin_function(builtins[i].func);
        nada_env_set(env, builtins[i].name, func);
        nada_free(func);  // Free the original after nada_env_set has made its copy
    }

    return env;
}

// Helper to check if a symbol is a built-in function
BuiltinFunc get_builtin_func(const char *name) {
    for (int i = 0; builtins[i].name != NULL; i++) {
        if (strcmp(name, builtins[i].name) == 0) {
            return builtins[i].func;
        }
    }
    return NULL;
}

// Helper to get builtin name from function
const char *get_builtin_name(BuiltinFunc func) {
    for (int i = 0; builtins[i].name != NULL; i++) {
        if (builtins[i].func == func) {
            return builtins[i].name;
        }
    }
    return NULL;
}

// Evaluate an expression in an environment
NadaValue *nada_eval(NadaValue *expr, NadaEnv *env) {
    // Self-evaluating expressions: numbers, strings, booleans, and nil
    if (expr->type == NADA_NUM || expr->type == NADA_STRING ||
        expr->type == NADA_BOOL || expr->type == NADA_NIL) {
        // Make a copy to avoid double-freeing
        switch (expr->type) {
        case NADA_NUM:
            return nada_create_num(expr->data.number);
        case NADA_STRING:
            return nada_create_string(expr->data.string);
        case NADA_BOOL:
            return nada_create_bool(expr->data.boolean);
        case NADA_NIL:
            return nada_create_nil();
        default:
            return nada_create_nil();  // Should never happen
        }
    }

    // Symbol lookup
    if (expr->type == NADA_SYMBOL) {
        return nada_env_get(env, expr->data.symbol, g_silent_symbol_lookup);
    }

    // List processing and the rest of the function...
    // List processing - function application
    if (expr->type == NADA_PAIR) {
        NadaValue *op = nada_car(expr);
        NadaValue *args = nada_cdr(expr);

        // Special forms
        if (op->type == NADA_SYMBOL) {
            // Quote special form
            if (strcmp(op->data.symbol, "quote") == 0) {
                return builtin_quote(args, env);
            }

            // Define special form
            if (strcmp(op->data.symbol, "define") == 0) {
                return builtin_define(args, env);
            }

            // Lambda special form
            if (strcmp(op->data.symbol, "lambda") == 0) {
                return builtin_lambda(args, env);
            }

            // Cond special form
            if (strcmp(op->data.symbol, "cond") == 0) {
                return builtin_cond(args, env);
            }

            // Let special form
            if (strcmp(op->data.symbol, "let") == 0) {
                return builtin_let(args, env);
            }

            // If special form
            if (strcmp(op->data.symbol, "if") == 0) {
                return builtin_if(args, env);
            }

            // Begin special form
            if (strcmp(op->data.symbol, "begin") == 0) {
                return builtin_begin(args, env);
            }

            // And special form
            if (strcmp(op->data.symbol, "and") == 0) {
                return builtin_and(args, env);
            }

            // Or special form
            if (strcmp(op->data.symbol, "or") == 0) {
                return builtin_or(args, env);
            }

            // Set! special form
            if (strcmp(op->data.symbol, "set!") == 0) {
                return builtin_set(args, env);
            }

            // Regular function application
            BuiltinFunc func = get_builtin_func(op->data.symbol);
            if (func != NULL) {
                return func(args, env);
            }

            // Try to apply as a user-defined function
            NadaValue *func_val = nada_env_get(env, op->data.symbol, 0);
            if (func_val->type == NADA_FUNC) {
                NadaValue *result = apply_function(func_val, args, env);
                nada_free(func_val);
                return result;
            }

            nada_free(func_val);
        }

        // Try to evaluate the operator position
        NadaValue *eval_op = nada_eval(op, env);
        if (eval_op->type == NADA_FUNC) {
            NadaValue *result = apply_function(eval_op, args, env);
            nada_free(eval_op);
            return result;
        }

        nada_free(eval_op);
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "not a function");
        return nada_create_nil();
    }

    // Default case
    nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "cannot evaluate expression");
    return nada_create_nil();
}

// In the initialization function (or main), add:
void nada_init(void) {
}

// Add to NadaEval.c
NadaValue *nada_create_builtin_function(NadaValue *(*func)(NadaValue *, NadaEnv *)) {
    NadaValue *val = malloc(sizeof(NadaValue));
    val->type = NADA_FUNC;
    val->data.function.params = NULL;   // No explicit parameters for builtins
    val->data.function.body = NULL;     // No body for builtins
    val->data.function.env = NULL;      // No closure environment
    val->data.function.builtin = func;  // Store the function pointer
    nada_increment_allocations();       // Move this AFTER initialization
    return val;
}

// Public helper to load a file
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
