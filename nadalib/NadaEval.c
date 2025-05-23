#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "NadaEval.h"
#include "NadaParser.h"
#include "NadaString.h"
#include "NadaError.h"
#include "NadaJupyter.h"

// Forward declaration of the builtins array
static BuiltinFuncInfo builtins[];

// From builtin special forms
void fix_env_references(NadaValue *value, NadaEnv *target_env, NadaEnv *replacement_env);

// Flag to control symbol error reporting
int g_silent_symbol_lookup = 0;

// Function to enable/disable silent symbol lookup
void nada_set_silent_symbol_lookup(int silent) {
    g_silent_symbol_lookup = silent;
}

bool nada_is_global_silent_symbol_lookup() {
    return g_silent_symbol_lookup;
}

// Enhance apply_function to handle function objects from lists
NadaValue *apply_function(NadaValue *func, NadaValue *args, NadaEnv *env) {
    // Special handling for built-in functions
    if (func->type == NADA_FUNC && func->data.function.builtin) {
        // Call the built-in function directly
        return func->data.function.builtin(args, env);
    }

    // For user-defined functions, create a new environment
    NadaEnv *func_env = nada_env_create(func->data.function.env);

    // Get parameter list and body
    NadaValue *params = func->data.function.params;
    NadaValue *body = func->data.function.body;

    // Handle variadic functions - find out if this is a variadic function
    int is_variadic = 0;
    const char *rest_param = NULL;

    // Check if params is a single symbol (fully variadic)
    if (params->type == NADA_SYMBOL) {
        is_variadic = 1;
        rest_param = params->data.symbol;
    }
    // Check if params ends with a dotted pair (partially variadic)
    else if (params->type == NADA_PAIR) {
        NadaValue *current = params;
        NadaValue *next;

        while (current->type == NADA_PAIR) {
            next = current->data.pair.cdr;
            if (next->type == NADA_SYMBOL && !nada_is_nil(next)) {
                is_variadic = 1;
                rest_param = next->data.symbol;
                break;
            }
            current = next;
        }
    }

    // Bind arguments to parameters
    if (is_variadic) {
        if (params->type == NADA_SYMBOL) {
            // Case: (lambda args body) - all args as a list
            // Evaluate each argument in the list
            NadaValue *evaluated_args = nada_create_nil();
            NadaValue *current = args;

            // Evaluate each argument and build a new list
            while (current->type == NADA_PAIR) {
                NadaValue *arg_evaluated = nada_eval(current->data.pair.car, env);
                NadaValue *new_args = nada_cons(arg_evaluated, evaluated_args);
                nada_free(arg_evaluated);   // Free after it's been copied
                nada_free(evaluated_args);  // Free the old list
                evaluated_args = new_args;  // Update our list pointer
                current = current->data.pair.cdr;
            }

            // Reverse the list to maintain the original order
            NadaValue *reversed_args = nada_reverse(evaluated_args);
            nada_free(evaluated_args);

            // Bind the evaluated arguments list to the parameter
            nada_env_set(func_env, rest_param, reversed_args);
            nada_free(reversed_args);  // Free after it's been stored
        } else {
            // Case: (lambda (a b . rest) body) - fixed args plus rest list
            NadaValue *current_param = params;
            NadaValue *current_arg = args;

            // Process the fixed parameters
            while (current_param->type == NADA_PAIR &&
                   current_param->data.pair.cdr->type == NADA_PAIR) {

                if (nada_is_nil(current_arg)) {
                    // Not enough arguments
                    nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "too few arguments");
                    nada_env_release(func_env);
                    return nada_create_nil();
                }

                // Evaluate and bind this parameter
                NadaValue *arg_evaluated = nada_eval(current_arg->data.pair.car, env);
                nada_env_set(func_env,
                             current_param->data.pair.car->data.symbol,
                             arg_evaluated);
                nada_free(arg_evaluated);  // Free after it's been stored

                // Move to next param and arg
                current_param = current_param->data.pair.cdr;
                current_arg = current_arg->data.pair.cdr;
            }

            // Bind the last fixed parameter
            if (current_param->type == NADA_PAIR &&
                current_param->data.pair.car->type == NADA_SYMBOL) {

                if (nada_is_nil(current_arg)) {
                    // Not enough arguments
                    nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "too few arguments");
                    nada_env_release(func_env);
                    return nada_create_nil();
                }

                // Bind this parameter
                nada_env_set(func_env,
                             current_param->data.pair.car->data.symbol,
                             current_arg->data.pair.car);

                // Move to next arg
                current_arg = current_arg->data.pair.cdr;
            }

            // Bind rest parameter to remaining args - don't use nada_deep_copy
            nada_env_set(func_env, rest_param, current_arg);
        }
    } else {
        // Regular function binding - same fix applies
        NadaValue *current_param = params;
        NadaValue *current_arg = args;

        while (!nada_is_nil(current_param) && !nada_is_nil(current_arg)) {
            if (current_param->type != NADA_PAIR || current_param->data.pair.car->type != NADA_SYMBOL) {
                nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "invalid parameter list");
                nada_env_release(func_env);
                return nada_create_nil();
            }

            // Evaluate the argument before binding it to the parameter
            NadaValue *arg_evaluated = nada_eval(current_arg->data.pair.car, env);
            nada_env_set(func_env,
                         current_param->data.pair.car->data.symbol,
                         arg_evaluated);
            nada_free(arg_evaluated);  // Free after it's been stored

            // Move to next param and arg
            current_param = current_param->data.pair.cdr;
            current_arg = current_arg->data.pair.cdr;
        }

        // Check for parameter/argument count mismatch
        if (!nada_is_nil(current_param)) {
            nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "too few arguments");
            nada_env_release(func_env);
            return nada_create_nil();
        }

        if (!nada_is_nil(current_arg)) {
            nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "too many arguments");
            nada_env_release(func_env);
            return nada_create_nil();
        }
    }

    // Evaluate body expressions
    NadaValue *result = nada_create_nil();
    NadaValue *current_expr = body;

    while (!nada_is_nil(current_expr)) {
        nada_free(result);
        result = nada_eval(current_expr->data.pair.car, func_env);
        current_expr = current_expr->data.pair.cdr;
    }

    // Make a deep copy before cleaning up
    NadaValue *result_copy = nada_deep_copy(result);
    nada_free(result);

    /*
    // IMPORTANT: Check if the result contains any functions that reference this environment
    if (result_copy->type == NADA_FUNC && result_copy->data.function.env == func_env) {
        // Break circular reference to prevent leak
        result_copy->data.function.env = func->data.function.env;
        if (func->data.function.env) {
            nada_env_add_ref(func->data.function.env);
        }
    } else if (result_copy->type == NADA_PAIR) {
        // Recursively scan list result for functions referencing this environment
        fix_env_references(result_copy, func_env, func->data.function.env);
    }
    */

    // Clean up
    nada_env_release(func_env);

    return result_copy;
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
        case NADA_ERROR:
            fprintf(stderr, "#<error: %s>", binding->value->data.error);
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

    // printf("Environment contents:\n");
    // print_bindings(env, 0);

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
    case NADA_ERROR:
        fprintf(f, "#<error: %s>", val->data.error);
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
void nada_serialize_env(NadaEnv *current_env, FILE *out) {
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

// Built-in function: error
static NadaValue *builtin_error(NadaValue *args, NadaEnv *env) {
    // Check that we have at least one argument
    if (nada_is_nil(args)) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "error requires at least one argument");
        return nada_create_error("error function called with no arguments");
    }

    // Evaluate the first argument
    NadaValue *message_val = nada_eval(nada_car(args), env);

    // Convert to string if needed
    char error_message[1024] = {0};

    if (message_val->type == NADA_STRING) {
        strncpy(error_message, message_val->data.string, sizeof(error_message) - 1);
    } else {
        // For other types, get a string representation
        char *temp = nada_value_to_string(message_val);
        if (temp) {
            strncpy(error_message, temp, sizeof(error_message) - 1);
            free(temp);
        } else {
            strcpy(error_message, "unknown error");
        }
    }

    // Report the error so it gets registered in the global error state
    nada_report_error(NADA_ERROR_TYPE_ERROR, "%s", error_message);

    // Free the evaluated argument
    nada_free(message_val);

    // Return an error value with the same message
    return nada_create_error(error_message);
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
    {"remainder", builtin_remainder},
    {"expt", builtin_expt},
    {"numerator", builtin_numerator},      // Add numerator function
    {"denominator", builtin_denominator},  // Add denominator function
    {"sign", builtin_sign},                // Add sign function
    {"factor", builtin_factor},            // Add factor function
    {"define", builtin_define},
    {"lambda", builtin_lambda},
    {"<", builtin_less_than},
    {"<=", builtin_less_equal},
    {">", builtin_greater_than},
    {">=", builtin_greater_equal},
    {"=", builtin_numeric_equal},
    {"eq?", builtin_eq},
    {"equal?", builtin_equal},

    // Add standard Scheme string comparison aliases
    {"string<?", builtin_less_than},
    {"string<=?", builtin_less_equal},
    {"string>?", builtin_greater_than},
    {"string>=?", builtin_greater_equal},
    {"string=?", builtin_eq},

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
    {"defined?", builtin_defined_p},
    {"boolean?", builtin_boolean_p},
    {"pair?", builtin_pair_p},
    {"function?", builtin_function_p},
    {"procedure?", builtin_procedure_p},
    {"list?", builtin_list_p},
    {"atom?", builtin_atom_p},
    {"builtin?", builtin_builtin_p},
    {"error?", builtin_error_p},

    // String operations
    {"string-length", builtin_string_length},
    {"substring", builtin_substring},
    {"string-split", builtin_string_split},
    {"string-join", builtin_string_join},
    {"string-upcase", builtin_string_upcase},
    {"string-downcase", builtin_string_downcase},
    {"string->number", builtin_string_to_number},
    {"number->string", builtin_number_to_string},
    {"float", builtin_float},
    {"tokenize-expr", builtin_tokenize_expr},  // Add this line
    {"read-from-string", builtin_read_from_string},
    {"write-to-string", builtin_write_to_string},
    {"string->symbol", builtin_string_to_symbol},  // Add this line

    // I/O operations
    {"read-file", builtin_read_file},
    {"write-file", builtin_write_file},
    {"display", builtin_display},
    {"display-markdown", builtin_display_markdown},
    {"display-html", builtin_display_html},
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

    // Add the new for-each function
    {"for-each", builtin_for_each},

    // Add the new set! function
    {"set!", builtin_set},

    {"apply", builtin_apply},

    {"error", builtin_error},

    {NULL, NULL}  // Sentinel to mark end of array
};

// Create a standard environment with basic operations
NadaEnv *nada_standard_env(void) {
    NadaEnv *env = nada_env_create(NULL);

    // Add built-in functions to environment
    for (int i = 0; builtins[i].name != NULL; i++) {
        // For simplicity, we're not creating proper function objects yet
        // We'll just use symbols with special handling in the evaluator
        NadaValue *symbol = nada_create_symbol(builtins[i].name);
        nada_env_set(env, builtins[i].name, symbol);
        // Free the symbol after it's been added to the environment
        nada_free(symbol);
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
    // Search through the existing builtins array for a matching name
    for (int i = 0; builtins[i].name != NULL; i++) {
        if (strcmp(name, builtins[i].name) == 0) {
            return builtins[i].func;
        }
    }

    // No match found
    return NULL;
}

// Enhanced function to get the name of a builtin function

const char *get_builtin_name(BuiltinFunc func) {
    // Add additional special case handling for arithmetic operators
    if (func == builtin_add) return "+";
    if (func == builtin_subtract) return "-";
    if (func == builtin_multiply) return "*";
    if (func == builtin_divide) return "/";

    // Loop through the builtins array to find a match
    for (int i = 0; builtins[i].name != NULL; i++) {
        if (builtins[i].func == func) {
            return builtins[i].name;
        }
    }

    // No match found
    return NULL;
}

// Evaluate an expression in an environment
NadaValue *nada_eval(NadaValue *expr, NadaEnv *env) {
    // Self-evaluating expressions: numbers, strings, booleans, nil, functions, and errors
    if (expr->type == NADA_NUM || expr->type == NADA_STRING ||
        expr->type == NADA_BOOL || expr->type == NADA_NIL ||
        expr->type == NADA_ERROR || expr->type == NADA_FUNC) {

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
        case NADA_ERROR:
            return nada_create_error(expr->data.error);
        case NADA_FUNC:
            // For built-in functions
            if (expr->data.function.builtin) {
                return nada_create_builtin_function(expr->data.function.builtin);
            }
            // For user-defined functions, use deep_copy instead of individual components
            else {
                return nada_deep_copy(expr);
            }
        default:
            return nada_create_nil();
        }
    }

    // Symbol lookup
    if (expr->type == NADA_SYMBOL) {
        return nada_env_get(env, expr->data.symbol, nada_is_global_silent_symbol_lookup());
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

        // Save the operator name for the error message
        char op_name[256] = "unknown";
        if (op->type == NADA_SYMBOL) {
            strncpy(op_name, op->data.symbol, sizeof(op_name) - 1);
            op_name[sizeof(op_name) - 1] = '\0';  // Ensure null termination
        } else if (eval_op->type == NADA_NIL) {
            strcpy(op_name, "nil");
        } else {
            // Try to get a representation of the value
            char *repr = nada_value_to_string(op);
            if (repr) {
                strncpy(op_name, repr, sizeof(op_name) - 1);
                op_name[sizeof(op_name) - 1] = '\0';
            }
        }

        nada_free(eval_op);
        if (!nada_is_global_silent_symbol_lookup()) {
            nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "'%s' is not a function", op_name);
        }
        return nada_create_nil();
    }

    // Default case
    nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "cannot evaluate expression");
    return nada_create_nil();
}

// In the initialization function (or main), add:
void nada_init(void) {
}

// eval: Evaluate a quoted expression
NadaValue *builtin_eval(NadaValue *args, NadaEnv *env) {
    // Check argument count
    if (nada_is_nil(args)) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "eval requires at least one argument");
        return nada_create_nil();
    }

    // Get the expression to evaluate (without evaluating it yet)
    NadaValue *expr = nada_car(args);
    NadaValue *rest_args = nada_cdr(args);

    // Standard 1-argument eval
    if (nada_is_nil(rest_args)) {
        NadaValue *expr_val = nada_eval(expr, env);

        // If the result is a list, evaluate it again
        if (expr_val->type == NADA_PAIR) {
            NadaValue *result = nada_eval(expr_val, env);
            nada_free(expr_val);
            return result;
        }

        return expr_val;
    }

    // Extended 3-argument eval with error handling
    // (eval expr error-handler success-handler)
    if (!nada_is_nil(nada_cdr(rest_args)) && nada_is_nil(nada_cdr(nada_cdr(rest_args)))) {
        // Get error and success handlers
        NadaValue *error_handler = nada_eval(nada_car(rest_args), env);
        NadaValue *success_handler = nada_eval(nada_car(nada_cdr(rest_args)), env);

        // Validate handlers are functions
        if (error_handler->type != NADA_FUNC || success_handler->type != NADA_FUNC) {
            nada_report_error(NADA_ERROR_TYPE_ERROR, "eval handlers must be functions");
            nada_free(error_handler);
            nada_free(success_handler);
            return nada_create_nil();
        }

        // Special case for symbols - check if symbol exists without triggering error
        if (expr->type == NADA_SYMBOL) {
            // Set silent lookup mode
            nada_set_silent_symbol_lookup(1);

            // Try to lookup the symbol silently
            NadaValue *lookup_result = nada_env_get(env, expr->data.symbol, 1);

            // Restore normal lookup mode
            nada_set_silent_symbol_lookup(0);

            // Check if symbol was found
            int symbol_found = !(lookup_result->type == NADA_NIL);
            nada_free(lookup_result);

            // If symbol wasn't found, call error handler
            if (!symbol_found) {
                // Call error handler with no arguments
                NadaValue *nil_args = nada_create_nil();
                NadaValue *result = apply_function(error_handler, nil_args, env);
                nada_free(nil_args);

                // Clean up
                nada_free(error_handler);
                nada_free(success_handler);

                return result;
            }
        }

        // Normal case - evaluate the expression
        NadaValue *eval_result = nada_eval(expr, env);

        // Call success handler with the result
        NadaValue *nil_val = nada_create_nil();
        NadaValue *handler_args = nada_cons(eval_result, nil_val);
        nada_free(nil_val);

        NadaValue *result = apply_function(success_handler, handler_args, env);

        // Clean up
        nada_free(eval_result);
        nada_free(handler_args);
        nada_free(error_handler);
        nada_free(success_handler);

        // CRITICAL FIX: Do NOT make a deep copy here - just return the result directly
        return result;
    }

    // Invalid argument count
    nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "eval takes 1 or 3 arguments");
    return nada_create_nil();
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
                NadaValue *new_tokens = nada_cons(token, tokens);  // Create new list with token
                nada_free(token);                                  // Free the token after it's been copied
                nada_free(tokens);                                 // Free the old list
                tokens = new_tokens;                               // Update our list pointer
                token_pos = 0;
            }

            // Add the operator token
            token_buf[0] = c;
            token_buf[1] = '\0';
            NadaValue *op_token = nada_create_string(token_buf);
            NadaValue *new_tokens = nada_cons(op_token, tokens);  // Create new list with op_token
            nada_free(op_token);                                  // Free the token after it's been copied
            nada_free(tokens);                                    // Free the old list
            tokens = new_tokens;                                  // Update our list pointer

        } else if (isdigit(c) || isalpha(c) || c == '.') {
            // Build number or variable tokens
            token_buf[token_pos++] = c;
        } else if (isspace(c)) {
            // Finish current token if any
            if (token_pos > 0) {
                token_buf[token_pos] = '\0';
                NadaValue *token = nada_create_string(token_buf);
                NadaValue *new_tokens = nada_cons(token, tokens);  // Create new list with token
                nada_free(token);                                  // Free the token after it's been copied
                nada_free(tokens);                                 // Free the old list
                tokens = new_tokens;                               // Update our list pointer
                token_pos = 0;
            }
        }
    }

    // Add any final token
    if (token_pos > 0) {
        token_buf[token_pos] = '\0';
        NadaValue *token = nada_create_string(token_buf);
        NadaValue *new_tokens = nada_cons(token, tokens);  // Create new list with token
        nada_free(token);                                  // Free the token after it's been copied
        nada_free(tokens);                                 // Free the old list
        tokens = new_tokens;                               // Update our list pointer
    }

    // Reverse the tokens list to get them in the original order
    NadaValue *result = nada_reverse(tokens);
    nada_free(tokens);   // Free the intermediate list
    nada_free(str_arg);  // Free the evaluated input string

    return result;
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
