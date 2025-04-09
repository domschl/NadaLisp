#include "NadaEval.h"
#include "NadaParser.h"
#include "NadaString.h"  // Add this include
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "NadaError.h"

// Add near the top of the file, after includes
extern void mark_evaluation_error();  // From run_lisp_tests.c

// Environment structure
struct NadaBinding {
    char *name;
    NadaValue *value;
    struct NadaBinding *next;
};

struct NadaEnv {
    struct NadaBinding *bindings;
    struct NadaEnv *parent;
};

// Type to represent a built-in function
typedef NadaValue *(*BuiltinFunc)(NadaValue *, NadaEnv *);

// Structure to hold built-in function info
typedef struct {
    const char *name;
    BuiltinFunc func;
} BuiltinFuncInfo;

// Forward declaration of the builtins array
static BuiltinFuncInfo builtins[];


// Create a new environment
NadaEnv *nada_env_create(NadaEnv *parent) {
    NadaEnv *env = malloc(sizeof(NadaEnv));
    env->bindings = NULL;
    env->parent = parent;
    return env;
}

// Free an environment and all its bindings
void nada_env_free(NadaEnv *env) {
    if (env == NULL) return;

    struct NadaBinding *current = env->bindings;
    while (current != NULL) {
        struct NadaBinding *next = current->next;
        free(current->name);
        nada_free(current->value);
        free(current);
        current = next;
    }

    free(env);
}

// Add a binding to the environment
void nada_env_set(NadaEnv *env, const char *name, NadaValue *value) {
    // Check if symbol already exists
    struct NadaBinding *current = env->bindings;
    while (current != NULL) {
        if (strcmp(current->name, name) == 0) {
            // Free the old value before replacing it
            nada_free(current->value);

            // Store a copy of the value (so caller can free original)
            current->value = nada_deep_copy(value);
            return;
        }
        current = current->next;
    }

    // Add new binding
    struct NadaBinding *new_binding = malloc(sizeof(struct NadaBinding));
    new_binding->name = strdup(name);
    new_binding->value = nada_deep_copy(value);
    new_binding->next = env->bindings;  // Add to front of list
    env->bindings = new_binding;
}

// Look up a binding in the environment
NadaValue *nada_env_get(NadaEnv *env, const char *name) {
    // Search in current environment
    struct NadaBinding *current = env->bindings;
    while (current != NULL) {
        if (strcmp(current->name, name) == 0) {
            // Return a deep copy of the value, not the original!
            return nada_deep_copy(current->value);
        }
        current = current->next;
    }

    // If not found and we have a parent, search there
    if (env->parent != NULL) {
        return nada_env_get(env->parent, name);
    }

    // Not found
    nada_report_error(NADA_ERROR_UNDEFINED_SYMBOL, "symbol '%s' not found in environment", name);
    return nada_create_nil();  // Return nil for undefined symbols
}

// First, add a function to remove a binding from the environment
void nada_env_remove(NadaEnv *env, const char *name) {
    struct NadaBinding *prev = NULL;
    struct NadaBinding *current = env->bindings;

    while (current != NULL) {
        if (strcmp(current->name, name) == 0) {
            // Found the binding to remove
            if (prev == NULL) {
                // It's the first binding
                env->bindings = current->next;
            } else {
                prev->next = current->next;
            }

            // Free the binding
            nada_free(current->value);
            free(current->name);
            free(current);
            return;
        }

        prev = current;
        current = current->next;
    }

    // If not found in current environment, try parent
    if (env->parent != NULL) {
        nada_env_remove(env->parent, name);
    }
}

// Built-in function: quote
static NadaValue *builtin_quote(NadaValue *args, NadaEnv *env) {
    // Check for exactly 1 argument
    if (nada_is_nil(args) || !nada_is_nil(nada_cdr(args))) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "quote requires exactly 1 argument");
        return nada_create_nil();
    }

    // Return unevaluated argument (make a deep copy to prevent modification)
    return nada_deep_copy(nada_car(args));
}

// Built-in function: car
static NadaValue *builtin_car(NadaValue *args, NadaEnv *env) {
    if (nada_is_nil(args) || !nada_is_nil(nada_cdr(args))) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "car requires exactly 1 argument");
        return nada_create_nil();
    }

    // Evaluate the argument
    NadaValue *val = nada_eval(nada_car(args), env);

    // Check that it's a pair
    if (val->type != NADA_PAIR) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "car called on non-pair");
        nada_free(val);
        return nada_create_nil();
    }

    // Return a copy of the car value to prevent modification
    NadaValue *result = nada_deep_copy(val->data.pair.car);
    nada_free(val);
    return result;
}

// Built-in function: cdr
static NadaValue *builtin_cdr(NadaValue *args, NadaEnv *env) {
    if (nada_is_nil(args) || !nada_is_nil(nada_cdr(args))) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "cdr takes exactly one argument");
        return nada_create_nil();
    }

    NadaValue *arg = nada_eval(nada_car(args), env);
    if (arg->type != NADA_PAIR) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "cdr requires a list argument");
        nada_free(arg);
        return nada_create_nil();
    }

    // Get the cdr and make a deep copy of it
    NadaValue *result = nada_deep_copy(nada_cdr(arg));

    // Free the evaluated argument
    nada_free(arg);

    return result;
}

// cadr: Get the second element of a list (car of cdr)
static NadaValue *builtin_cadr(NadaValue *args, NadaEnv *env) {
    if (nada_is_nil(args) || !nada_is_nil(nada_cdr(args))) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "cadr requires exactly 1 argument");
        return nada_create_nil();
    }

    // Evaluate the argument
    NadaValue *list_arg = nada_eval(nada_car(args), env);

    // Check that it's a pair
    if (list_arg->type != NADA_PAIR) {
        nada_report_error(NADA_ERROR_TYPE_ERROR, "cadr requires a list argument");
        nada_free(list_arg);
        return nada_create_nil();
    }

    // Get the cdr
    NadaValue *cdr_val = nada_cdr(list_arg);

    // Check that cdr is a pair
    if (cdr_val->type != NADA_PAIR) {
        nada_report_error(NADA_ERROR_TYPE_ERROR, "list has no second element");
        nada_free(list_arg);
        return nada_create_nil();
    }

    // Get the car of cdr (second element)
    NadaValue *result = nada_deep_copy(nada_car(cdr_val));

    // Clean up
    nada_free(list_arg);

    return result;
}

// caddr: Get the third element of a list (car of cdr of cdr)
static NadaValue *builtin_caddr(NadaValue *args, NadaEnv *env) {
    if (nada_is_nil(args) || !nada_is_nil(nada_cdr(args))) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "caddr requires exactly 1 argument");
        return nada_create_nil();
    }

    // Evaluate the argument
    NadaValue *list_arg = nada_eval(nada_car(args), env);

    // Check that it's a pair
    if (list_arg->type != NADA_PAIR) {
        nada_report_error(NADA_ERROR_TYPE_ERROR, "caddr requires a list argument");
        nada_free(list_arg);
        return nada_create_nil();
    }

    // Get the cdr
    NadaValue *cdr_val = nada_cdr(list_arg);

    // Check that cdr is a pair
    if (cdr_val->type != NADA_PAIR) {
        nada_report_error(NADA_ERROR_TYPE_ERROR, "list has no second element");
        nada_free(list_arg);
        return nada_create_nil();
    }

    // Get the cdr of cdr
    NadaValue *cddr_val = nada_cdr(cdr_val);

    // Check that cddr is a pair
    if (cddr_val->type != NADA_PAIR) {
        nada_report_error(NADA_ERROR_TYPE_ERROR, "list has no third element");
        nada_free(list_arg);
        return nada_create_nil();
    }

    // Get the car of cdr of cdr (third element)
    NadaValue *result = nada_deep_copy(nada_car(cddr_val));

    // Clean up
    nada_free(list_arg);

    return result;
}

// Addition (+)
static NadaValue *builtin_add(NadaValue *args, NadaEnv *env) {
    if (nada_is_nil(args)) {
        return nada_create_num_from_int(0);
    }

    // Start with first argument
    NadaValue *first_arg = nada_eval(nada_car(args), env);
    if (first_arg->type != NADA_NUM) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "'+' requires number arguments");
        nada_free(first_arg);
        return nada_create_num_from_int(0);
    }

    char *num_str = nada_num_to_string(first_arg->data.number);
    free(num_str);

    NadaNum *result = nada_num_copy(first_arg->data.number);
    nada_free(first_arg);

    // Add the rest of the arguments
    NadaValue *current = nada_cdr(args);
    while (!nada_is_nil(current)) {
        NadaValue *arg = nada_eval(nada_car(current), env);
        if (arg->type != NADA_NUM) {
            nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "'+' requires number arguments");
            nada_num_free(result);
            nada_free(arg);
            return nada_create_num_from_int(0);
        }

        char *arg_str = nada_num_to_string(arg->data.number);
        free(arg_str);

        NadaNum *temp = nada_num_add(result, arg->data.number);

        char *temp_str = nada_num_to_string(temp);
        free(temp_str);

        nada_num_free(result);
        result = temp;

        nada_free(arg);
        current = nada_cdr(current);
    }

    NadaValue *val = nada_create_num(result);
    nada_num_free(result);
    return val;
}

// Subtraction (-)
static NadaValue *builtin_subtract(NadaValue *args, NadaEnv *env) {
    if (nada_is_nil(args)) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "'-' requires at least one argument");
        return nada_create_num_from_int(0);
    }

    NadaValue *first = nada_eval(nada_car(args), env);
    if (first->type != NADA_NUM) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "'-' requires number arguments");
        nada_free(first);
        return nada_create_num_from_int(0);
    }

    NadaNum *result = nada_num_copy(first->data.number);
    nada_free(first);

    NadaValue *rest = nada_cdr(args);
    if (nada_is_nil(rest)) {
        // Unary minus
        NadaNum *neg = nada_num_negate(result);
        nada_num_free(result);
        NadaValue *val = nada_create_num(neg);
        nada_num_free(neg);
        return val;
    }

    // Binary subtraction
    while (!nada_is_nil(rest)) {
        NadaValue *arg = nada_eval(nada_car(rest), env);
        if (arg->type != NADA_NUM) {
            nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "'-' requires number arguments");
            nada_num_free(result);
            nada_free(arg);
            return nada_create_num_from_int(0);
        }

        NadaNum *temp = nada_num_subtract(result, arg->data.number);
        nada_num_free(result);
        result = temp;

        nada_free(arg);
        rest = nada_cdr(rest);
    }

    NadaValue *val = nada_create_num(result);
    nada_num_free(result);
    return val;
}

// Multiplication (*)
static NadaValue *builtin_multiply(NadaValue *args, NadaEnv *env) {
    if (nada_is_nil(args)) {
        // Multiplication with no args is 1
        return nada_create_num_from_int(1);
    }

    // Start with first argument
    NadaValue *first_arg = nada_eval(nada_car(args), env);
    if (first_arg->type != NADA_NUM) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "'*' requires number arguments");
        nada_free(first_arg);
        return nada_create_num_from_int(0);
    }

    NadaNum *result = nada_num_copy(first_arg->data.number);
    nada_free(first_arg);

    // Multiply by the rest of the arguments
    NadaValue *current = nada_cdr(args);
    while (!nada_is_nil(current)) {
        NadaValue *arg = nada_eval(nada_car(current), env);
        if (arg->type != NADA_NUM) {
            nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "'*' requires number arguments");
            nada_num_free(result);
            nada_free(arg);
            return nada_create_num_from_int(0);
        }

        NadaNum *temp = nada_num_multiply(result, arg->data.number);
        nada_num_free(result);
        result = temp;

        nada_free(arg);
        current = nada_cdr(current);
    }

    NadaValue *val = nada_create_num(result);
    nada_num_free(result);
    return val;
}

// Division (/)
static NadaValue *builtin_divide(NadaValue *args, NadaEnv *env) {
    if (nada_is_nil(args)) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "'/' requires at least one argument");
        return nada_create_num_from_int(0);
    }

    NadaValue *first = nada_eval(nada_car(args), env);
    if (first->type != NADA_NUM) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "'/' requires number arguments");
        nada_free(first);
        return nada_create_num_from_int(0);
    }

    NadaNum *result = nada_num_copy(first->data.number);
    nada_free(first);

    NadaValue *rest = nada_cdr(args);
    if (nada_is_nil(rest)) {
        // Unary division (1/x)
        if (nada_num_is_zero(result)) {
            nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "division by zero");
            nada_num_free(result);
            return nada_create_num_from_int(0);
        }

        NadaNum *one = nada_num_from_int(1);
        NadaNum *temp = nada_num_divide(one, result);
        nada_num_free(one);
        nada_num_free(result);

        NadaValue *val = nada_create_num(temp);
        nada_num_free(temp);
        return val;
    }

    // Binary division
    while (!nada_is_nil(rest)) {
        NadaValue *arg = nada_eval(nada_car(rest), env);
        if (arg->type != NADA_NUM) {
            nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "'/' requires number arguments");
            nada_num_free(result);
            nada_free(arg);
            return nada_create_num_from_int(0);
        }

        if (nada_num_is_zero(arg->data.number)) {
            nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "division by zero");
            nada_num_free(result);
            nada_free(arg);
            return nada_create_num_from_int(0);
        }

        NadaNum *temp = nada_num_divide(result, arg->data.number);
        nada_num_free(result);
        result = temp;

        nada_free(arg);
        rest = nada_cdr(rest);
    }

    NadaValue *val = nada_create_num(result);
    nada_num_free(result);
    return val;
}

// Modulo (%)
static NadaValue *builtin_modulo(NadaValue *args, NadaEnv *env) {
    if (nada_is_nil(args) || nada_is_nil(nada_cdr(args)) ||
        !nada_is_nil(nada_cdr(nada_cdr(args)))) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "modulo requires exactly 2 arguments");
        return nada_create_num_from_int(0);
    }

    NadaValue *first = nada_eval(nada_car(args), env);
    NadaValue *second = nada_eval(nada_car(nada_cdr(args)), env);

    if (first->type != NADA_NUM || second->type != NADA_NUM) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "modulo requires number arguments");
        nada_free(first);
        nada_free(second);
        return nada_create_num_from_int(0);
    }

    if (nada_num_is_zero(second->data.number)) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "modulo by zero");
        nada_free(first);
        nada_free(second);
        return nada_create_num_from_int(0);
    }

    NadaNum *result = nada_num_modulo(first->data.number, second->data.number);

    nada_free(first);
    nada_free(second);

    NadaValue *val = nada_create_num(result);
    nada_num_free(result);
    return val;
}

// Built-in special form: define
static NadaValue *builtin_define(NadaValue *args, NadaEnv *env) {
    // Check that we have at least two arguments
    if (nada_is_nil(args) || nada_is_nil(nada_cdr(args))) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "define requires at least 2 arguments");
        return nada_create_nil();
    }

    NadaValue *first = nada_car(args);

    // Case 1: Define a variable - (define symbol expr)
    if (first->type == NADA_SYMBOL) {
        // This is the existing implementation
        NadaValue *val_expr = nada_car(nada_cdr(args));
        NadaValue *val = nada_eval(val_expr, env);
        nada_env_set(env, first->data.symbol, val);
        nada_free(val);  // Free the value after it's been stored
        return nada_create_symbol(first->data.symbol);
    }

    // Case 2: Define a function - (define (func-name args...) body...)
    if (first->type == NADA_PAIR && nada_car(first)->type == NADA_SYMBOL) {
        // Extract function name
        NadaValue *func_name = nada_car(first);

        // Extract parameters (rest of the first list)
        NadaValue *params = nada_cdr(first);

        // Extract body (rest of the args)
        NadaValue *body = nada_cdr(args);

        // Create a function value
        NadaValue *func = nada_create_function(
            nada_deep_copy(params),
            nada_deep_copy(body),
            env);

        // Bind function to name
        nada_env_set(env, func_name->data.symbol, func);
        
        // Free the function value after it's been stored in the environment
        nada_free(func);

        // Return the function name
        return nada_create_symbol(func_name->data.symbol);
    }

    nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "invalid define syntax");
    return nada_create_nil();
}

// Built-in special form: lambda
static NadaValue *builtin_lambda(NadaValue *args, NadaEnv *env) {
    // Check that we have at least two arguments: params and body
    if (nada_is_nil(args) || nada_is_nil(nada_cdr(args))) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "lambda requires parameters and body");
        return nada_create_nil();
    }

    // First argument must be a list of symbols (parameter names)
    NadaValue *params = nada_car(args);
    // Validate parameters (should all be symbols)
    NadaValue *param_check = params;
    while (param_check->type == NADA_PAIR) {
        if (nada_car(param_check)->type != NADA_SYMBOL) {
            nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "lambda parameters must be symbols");
            return nada_create_nil();
        }
        param_check = nada_cdr(param_check);
    }

    // The rest is the function body
    NadaValue *body = nada_cdr(args);

    // Create and return a new function value
    return nada_create_function(
        nada_deep_copy(params),
        nada_deep_copy(body),
        env  // Capture the current environment
    );
}

// Built-in special form: cond
static NadaValue *builtin_cond(NadaValue *args, NadaEnv *env) {
    // Check for empty cond
    if (nada_is_nil(args)) {
        return nada_create_nil();
    }

    // Process each clause
    NadaValue *clauses = args;

    while (!nada_is_nil(clauses)) {
        // Get current clause
        NadaValue *clause = nada_car(clauses);
        NadaValue *next_clauses = nada_cdr(clauses);

        // Each clause should be a list
        if (clause->type != NADA_PAIR) {
            nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "cond clause must be a list");
            return nada_create_nil();
        }

        // Get the condition
        NadaValue *condition = nada_car(clause);

        // Handle 'else' keyword (must be the last clause)
        int is_else = (condition->type == NADA_SYMBOL &&
                       strcmp(condition->data.symbol, "else") == 0);

        if (is_else) {
            // Verify this is the last clause
            if (!nada_is_nil(next_clauses)) {
                nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "'else' must be in the last cond clause");
                return nada_create_nil();
            }

            // Treat else as true without evaluation
            NadaValue *body = nada_cdr(clause);

            // If body is empty, return true
            if (nada_is_nil(body)) {
                return nada_create_bool(1);
            }

            // Evaluate each expression in the body, returning the last result
            NadaValue *result = nada_create_nil();

            while (!nada_is_nil(body)) {
                // Free previous intermediate result
                nada_free(result);

                // Evaluate next expression
                result = nada_eval(nada_car(body), env);
                body = nada_cdr(body);
            }

            return result;
        }

        // Normal clause - evaluate the condition
        NadaValue *test_result = nada_eval(condition, env);

        // Check if condition is true (anything other than #f or nil)
        int is_true = !(test_result->type == NADA_BOOL && test_result->data.boolean == 0) &&
                      !(test_result->type == NADA_NIL);

        // Free the test result
        nada_free(test_result);

        if (is_true) {
            // Get the body (rest of the clause)
            NadaValue *body = nada_cdr(clause);

            // If body is empty, return true
            if (nada_is_nil(body)) {
                return nada_create_bool(1);
            }

            // Evaluate each expression in the body, returning the last result
            NadaValue *result = nada_create_nil();

            while (!nada_is_nil(body)) {
                // Always free previous intermediate result
                nada_free(result);

                // Evaluate next expression
                result = nada_eval(nada_car(body), env);
                body = nada_cdr(body);
            }

            return result;
        }

        // Move to next clause
        clauses = next_clauses;
    }

    // No condition matched - return nil
    return nada_create_nil();
}

// Built-in special form: let (with support for named let)
static NadaValue *builtin_let(NadaValue *args, NadaEnv *env) {
    // Check we have at least bindings and body
    if (nada_is_nil(args) || nada_is_nil(nada_cdr(args))) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "let requires bindings and body");
        return nada_create_nil();
    }

    // Get bindings list or loop name
    NadaValue *first_arg = nada_car(args);

    // Check if this is a named let
    if (first_arg->type == NADA_SYMBOL) {
        // Named let: (let loop ((var1 val1)...) body...)
        char *loop_name = first_arg->data.symbol;
        NadaValue *bindings = nada_car(nada_cdr(args));
        NadaValue *body = nada_cdr(nada_cdr(args));

        // Bindings must be a list
        if (!nada_is_nil(bindings) && bindings->type != NADA_PAIR) {
            nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "let bindings must be a list");
            return nada_create_nil();
        }

        // Create a new environment
        NadaEnv *let_env = nada_env_create(env);

        // Extract parameters and initial values
        NadaValue *params = nada_create_nil();
        NadaValue *param_names = nada_create_nil();
        NadaValue *current_binding = bindings;

        while (!nada_is_nil(current_binding)) {
            NadaValue *binding = nada_car(current_binding);

            // Each binding must be a pair (var val)
            if (binding->type != NADA_PAIR ||
                nada_car(binding)->type != NADA_SYMBOL ||
                nada_is_nil(nada_cdr(binding)) ||
                !nada_is_nil(nada_cdr(nada_cdr(binding)))) {
                nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "let binding must be a (variable value) pair");
                nada_env_free(let_env);
                nada_free(params);
                nada_free(param_names);
                return nada_create_nil();
            }

            // Build parameter list for the function
            NadaValue *param = nada_car(binding);
            param_names = nada_cons(param, param_names);

            // Evaluate initial values
            NadaValue *val_expr = nada_car(nada_cdr(binding));
            NadaValue *val = nada_eval(val_expr, env);
            params = nada_cons(val, params);

            current_binding = nada_cdr(current_binding);
        }

        // Reverse parameters to match order
        NadaValue *reversed_params = nada_create_nil();
        NadaValue *reversed_names = nada_create_nil();

        while (!nada_is_nil(params)) {
            reversed_params = nada_cons(nada_car(params), reversed_params);
            params = nada_cdr(params);
        }

        while (!nada_is_nil(param_names)) {
            reversed_names = nada_cons(nada_car(param_names), reversed_names);
            param_names = nada_cdr(param_names);
        }

        // Create a lambda for the recursive function
        NadaValue *lambda = nada_create_function(reversed_names, body, let_env);

        // Bind the function in its own environment
        nada_env_set(let_env, loop_name, lambda);

        // Call the function with initial parameters
        NadaValue *call = nada_cons(nada_create_symbol(loop_name), reversed_params);
        NadaValue *result = nada_eval(call, let_env);

        // Clean up
        nada_free(call);
        nada_env_free(let_env);

        return result;
    } else {
        // Regular let: (let ((var1 val1)...) body...)
        NadaValue *bindings = first_arg;

        // Bindings must be a list
        if (!nada_is_nil(bindings) && bindings->type != NADA_PAIR) {
            nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "let bindings must be a list");
            return nada_create_nil();
        }

        // Create a new environment with the parent as the current environment
        NadaEnv *let_env = nada_env_create(env);

        // Process each binding
        NadaValue *binding_list = bindings;
        while (!nada_is_nil(binding_list)) {
            NadaValue *binding = nada_car(binding_list);

            // Each binding must be a pair (var val)
            if (binding->type != NADA_PAIR ||
                nada_car(binding)->type != NADA_SYMBOL ||
                nada_is_nil(nada_cdr(binding)) ||
                !nada_is_nil(nada_cdr(nada_cdr(binding)))) {
                nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "let binding must be a (variable value) pair");
                nada_env_free(let_env);
                return nada_create_nil();
            }

            // Get the variable name and value
            char *var_name = nada_car(binding)->data.symbol;
            NadaValue *val_expr = nada_car(nada_cdr(binding));

            // Evaluate the value in the original environment
            NadaValue *val = nada_eval(val_expr, env);

            // Bind it in the new environment
            nada_env_set(let_env, var_name, val);
            nada_free(val);

            binding_list = nada_cdr(binding_list);
        }

        // Evaluate the body in the new environment
        NadaValue *body = nada_cdr(args);
        NadaValue *result = nada_create_nil();

        // Evaluate each form in the body, returning the last result
        while (!nada_is_nil(body)) {
            nada_free(result);
            NadaValue *expr = nada_car(body);
            result = nada_eval(expr, let_env);
            body = nada_cdr(body);
        }

        nada_env_free(let_env);
        return result;
    }
}

// Apply a function to arguments
static NadaValue *apply_function(NadaValue *func, NadaValue *args, NadaEnv *env) {
    if (func->type != NADA_FUNC) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "attempt to apply non-function");
        return nada_create_nil();
    }

    // Handle built-in functions
    if (func->data.function.builtin != NULL) {
        // For a built-in function, we need to evaluate the arguments
        // before passing them to the function, as the built-in function
        // expects evaluated values

        // Create a list to hold evaluated arguments
        NadaValue *eval_args = nada_create_nil();
        NadaValue *current_arg = args;

        // Evaluate each argument
        while (!nada_is_nil(current_arg)) {
            // Evaluate the current argument
            NadaValue *arg_val = nada_eval(nada_car(current_arg), env);

            // Prepend to our list (we'll reverse it later)
            eval_args = nada_cons(arg_val, eval_args);
            nada_free(arg_val);

            // Move to next argument
            current_arg = nada_cdr(current_arg);
        }

        // Reverse the list to get arguments in the correct order
        NadaValue *reversed_args = nada_create_nil();
        current_arg = eval_args;

        while (!nada_is_nil(current_arg)) {
            reversed_args = nada_cons(nada_car(current_arg), reversed_args);
            current_arg = nada_cdr(current_arg);
        }

        // Call the built-in function with evaluated arguments
        NadaValue *result = func->data.function.builtin(reversed_args, env);

        // Free our intermediate lists
        nada_free(eval_args);
        nada_free(reversed_args);

        return result;
    }

    // Create a new environment with the function's environment as parent
    NadaEnv *func_env = nada_env_create(func->data.function.env);

    // Evaluate and bind arguments to parameters
    NadaValue *param = func->data.function.params;
    NadaValue *arg = args;

    while (!nada_is_nil(param) && !nada_is_nil(arg)) {
        // Get parameter name
        NadaValue *param_name = nada_car(param);

        // Evaluate argument
        NadaValue *arg_val = nada_eval(nada_car(arg), env);

        // Bind parameter to argument value
        nada_env_set(func_env, param_name->data.symbol, arg_val);

        // Free the evaluated argument value after it's been copied
        nada_free(arg_val);

        // Move to next parameter and argument
        param = nada_cdr(param);
        arg = nada_cdr(arg);
    }

    // Check for argument count mismatches
    if (!nada_is_nil(param)) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "too few arguments");
        nada_env_free(func_env);
        return nada_create_nil();
    }

    if (!nada_is_nil(arg)) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "too many arguments");
        nada_env_free(func_env);
        return nada_create_nil();
    }

    // Evaluate body expressions in sequence, returning the last result
    NadaValue *result = nada_create_nil();
    NadaValue *body = func->data.function.body;

    if (nada_is_nil(body)) {
        // If body is empty, return the initial nil result
        // (no need to free it since we're returning it)
        nada_env_free(func_env);
        return result;
    }

    // Process body expressions
    while (!nada_is_nil(body)) {
        // Free the previous result before replacing it
        // Now we unconditionally free the result, even if it's nil
        nada_free(result);

        // Evaluate the next expression in the body
        result = nada_eval(nada_car(body), func_env);
        body = nada_cdr(body);
    }

    // Clean up the function environment
    nada_env_free(func_env);

    return result;
}

// Less than (<)
static NadaValue *builtin_less_than(NadaValue *args, NadaEnv *env) {
    if (nada_is_nil(args) || nada_is_nil(nada_cdr(args)) ||
        !nada_is_nil(nada_cdr(nada_cdr(args)))) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "< requires exactly 2 arguments");
        return nada_create_bool(0);
    }

    NadaValue *first = nada_eval(nada_car(args), env);
    NadaValue *second = nada_eval(nada_car(nada_cdr(args)), env);

    if (first->type != NADA_NUM || second->type != NADA_NUM) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "< requires number arguments");
        nada_free(first);
        nada_free(second);
        return nada_create_bool(0);
    }

    bool result = nada_num_less(first->data.number, second->data.number);

    nada_free(first);
    nada_free(second);

    return nada_create_bool(result);
}

// Less than or equal (<=)
static NadaValue *builtin_less_equal(NadaValue *args, NadaEnv *env) {
    if (nada_is_nil(args) || nada_is_nil(nada_cdr(args)) ||
        !nada_is_nil(nada_cdr(nada_cdr(args)))) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "<= requires exactly 2 arguments");
        return nada_create_bool(0);
    }

    NadaValue *first = nada_eval(nada_car(args), env);
    NadaValue *second = nada_eval(nada_car(nada_cdr(args)), env);

    if (first->type != NADA_NUM || second->type != NADA_NUM) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "<= requires number arguments");
        nada_free(first);
        nada_free(second);
        return nada_create_bool(0);
    }

    bool result = nada_num_less_equal(first->data.number, second->data.number);

    nada_free(first);
    nada_free(second);

    return nada_create_bool(result);
}

// Greater than (>)
static NadaValue *builtin_greater_than(NadaValue *args, NadaEnv *env) {
    if (nada_is_nil(args) || nada_is_nil(nada_cdr(args)) ||
        !nada_is_nil(nada_cdr(nada_cdr(args)))) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "> requires exactly 2 arguments");
        return nada_create_bool(0);
    }

    NadaValue *first = nada_eval(nada_car(args), env);
    NadaValue *second = nada_eval(nada_car(nada_cdr(args)), env);

    if (first->type != NADA_NUM || second->type != NADA_NUM) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "> requires number arguments");
        nada_free(first);
        nada_free(second);
        return nada_create_bool(0);
    }

    bool result = nada_num_greater(first->data.number, second->data.number);

    nada_free(first);
    nada_free(second);

    return nada_create_bool(result);
}

// Greater than or equal (>=)
static NadaValue *builtin_greater_equal(NadaValue *args, NadaEnv *env) {
    if (nada_is_nil(args) || nada_is_nil(nada_cdr(args)) ||
        !nada_is_nil(nada_cdr(nada_cdr(args)))) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT, ">= requires exactly 2 arguments");
        return nada_create_bool(0);
    }

    NadaValue *first = nada_eval(nada_car(args), env);
    NadaValue *second = nada_eval(nada_car(nada_cdr(args)), env);

    if (first->type != NADA_NUM || second->type != NADA_NUM) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT, ">= requires number arguments");
        nada_free(first);
        nada_free(second);
        return nada_create_bool(0);
    }

    bool result = nada_num_greater_equal(first->data.number, second->data.number);

    nada_free(first);
    nada_free(second);

    return nada_create_bool(result);
}

// Numeric equality (=)
static NadaValue *builtin_numeric_equal(NadaValue *args, NadaEnv *env) {
    if (nada_is_nil(args) || nada_is_nil(nada_cdr(args)) ||
        !nada_is_nil(nada_cdr(nada_cdr(args)))) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "= requires exactly 2 arguments");
        return nada_create_bool(0);
    }

    NadaValue *first = nada_eval(nada_car(args), env);
    NadaValue *second = nada_eval(nada_car(nada_cdr(args)), env);

    if (first->type != NADA_NUM || second->type != NADA_NUM) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "= requires number arguments");
        nada_free(first);
        nada_free(second);
        return nada_create_bool(0);
    }

    bool result = nada_num_equal(first->data.number, second->data.number);

    nada_free(first);
    nada_free(second);

    return nada_create_bool(result);
}

// Identity equality (eq?)
static NadaValue *builtin_eq(NadaValue *args, NadaEnv *env) {
    if (nada_is_nil(args) || nada_is_nil(nada_cdr(args)) ||
        !nada_is_nil(nada_cdr(nada_cdr(args)))) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "eq? requires exactly 2 arguments");
        return nada_create_bool(0);
    }

    NadaValue *first = nada_eval(nada_car(args), env);
    NadaValue *second = nada_eval(nada_car(nada_cdr(args)), env);

    int result = 0;

    // Must be the same type to be eq?
    if (first->type == second->type) {
        switch (first->type) {
        case NADA_NUM:
            // For rational numbers, compare using nada_num_equal
            result = nada_num_equal(first->data.number, second->data.number);
            break;
        case NADA_BOOL:
            result = (first->data.boolean == second->data.boolean);
            break;
        case NADA_STRING:
            // For strings, compare the contents (interned strings)
            result = (strcmp(first->data.string, second->data.string) == 0);
            break;
        case NADA_SYMBOL:
            // Symbols with the same name are eq?
            result = (strcmp(first->data.symbol, second->data.symbol) == 0);
            break;
        case NADA_NIL:
            // All empty lists are eq?
            result = 1;
            break;
        case NADA_PAIR:
        case NADA_FUNC:
            // For pairs and functions, they need to be the same object
            // This simplified implementation always returns false
            result = 0;
            break;
        }
    }

    nada_free(first);
    nada_free(second);

    return nada_create_bool(result);
}

// Helper function for recursive equality check
static int values_equal(NadaValue *a, NadaValue *b) {
    if (a->type != b->type) return 0;

    switch (a->type) {
    case NADA_NUM:
        return nada_num_equal(a->data.number, b->data.number);
    case NADA_BOOL:
        return a->data.boolean == b->data.boolean;
    case NADA_STRING:
        return strcmp(a->data.string, b->data.string) == 0;
    case NADA_SYMBOL:
        return strcmp(a->data.symbol, b->data.symbol) == 0;
    case NADA_NIL:
        return 1;
    case NADA_PAIR:
        // Recursively check car and cdr
        return values_equal(a->data.pair.car, b->data.pair.car) &&
               values_equal(a->data.pair.cdr, b->data.pair.cdr);
    case NADA_FUNC:
        // Functions are complex; for simplicity, consider them not equal
        return 0;
    default:
        return 0;
    }
}

// Recursive structural equality (equal?)
static NadaValue *builtin_equal(NadaValue *args, NadaEnv *env) {
    if (nada_is_nil(args) || nada_is_nil(nada_cdr(args)) ||
        !nada_is_nil(nada_cdr(nada_cdr(args)))) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "equal? requires exactly 2 arguments");
        return nada_create_bool(0);
    }

    NadaValue *first = nada_eval(nada_car(args), env);
    NadaValue *second = nada_eval(nada_car(nada_cdr(args)), env);

    int result = values_equal(first, second);

    nada_free(first);
    nada_free(second);

    return nada_create_bool(result);
}

// Empty list test (null?)
static NadaValue *builtin_null(NadaValue *args, NadaEnv *env) {
    if (nada_is_nil(args) || !nada_is_nil(nada_cdr(args))) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "null? requires exactly 1 argument");
        return nada_create_bool(0);
    }

    NadaValue *arg = nada_eval(nada_car(args), env);
    int result = (arg->type == NADA_NIL);

    nada_free(arg);

    return nada_create_bool(result);
}

// Integer predicate (integer?)
static NadaValue *builtin_integer_p(NadaValue *args, NadaEnv *env) {
    if (nada_is_nil(args) || !nada_is_nil(nada_cdr(args))) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "integer? requires exactly 1 argument");
        return nada_create_bool(0);
    }

    NadaValue *val = nada_eval(nada_car(args), env);
    int result = (val->type == NADA_NUM && nada_num_is_integer(val->data.number));
    nada_free(val);
    return nada_create_bool(result);
}

// Number predicate (number?)
static NadaValue *builtin_number_p(NadaValue *args, NadaEnv *env) {
    if (nada_is_nil(args) || !nada_is_nil(nada_cdr(args))) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "number? requires exactly 1 argument");
        return nada_create_bool(0);
    }

    NadaValue *val = nada_eval(nada_car(args), env);
    int result = (val->type == NADA_NUM);
    nada_free(val);
    return nada_create_bool(result);
}

// String predicate (string?)
static NadaValue *builtin_string_p(NadaValue *args, NadaEnv *env) {
    if (nada_is_nil(args) || !nada_is_nil(nada_cdr(args))) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "string? requires exactly 1 argument");
        return nada_create_bool(0);
    }

    NadaValue *val = nada_eval(nada_car(args), env);
    int result = (val->type == NADA_STRING);
    nada_free(val);
    return nada_create_bool(result);
}

// Symbol predicate (symbol?)
static NadaValue *builtin_symbol_p(NadaValue *args, NadaEnv *env) {
    if (nada_is_nil(args) || !nada_is_nil(nada_cdr(args))) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "symbol? requires exactly 1 argument");
        return nada_create_bool(0);
    }

    NadaValue *val = nada_eval(nada_car(args), env);
    int result = (val->type == NADA_SYMBOL);
    nada_free(val);
    return nada_create_bool(result);
}

// Boolean predicate (boolean?)
static NadaValue *builtin_boolean_p(NadaValue *args, NadaEnv *env) {
    if (nada_is_nil(args) || !nada_is_nil(nada_cdr(args))) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "boolean? requires exactly 1 argument");
        return nada_create_bool(0);
    }

    NadaValue *val = nada_eval(nada_car(args), env);
    int result = (val->type == NADA_BOOL);
    nada_free(val);
    return nada_create_bool(result);
}

// Pair predicate (pair?)
static NadaValue *builtin_pair_p(NadaValue *args, NadaEnv *env) {
    if (nada_is_nil(args) || !nada_is_nil(nada_cdr(args))) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "pair? requires exactly 1 argument");
        return nada_create_bool(0);
    }

    NadaValue *val = nada_eval(nada_car(args), env);
    int result = (val->type == NADA_PAIR);
    nada_free(val);
    return nada_create_bool(result);
}

// Function predicate (function?)
static NadaValue *builtin_function_p(NadaValue *args, NadaEnv *env) {
    if (nada_is_nil(args) || !nada_is_nil(nada_cdr(args))) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "function? requires exactly 1 argument");
        return nada_create_bool(0);
    }

    NadaValue *val = nada_eval(nada_car(args), env);
    int result = (val->type == NADA_FUNC);
    nada_free(val);
    return nada_create_bool(result);
}

// Helper function to check if a value is a proper list
static int is_proper_list(NadaValue *v) {
    if (v->type == NADA_NIL) return 1;
    if (v->type != NADA_PAIR) return 0;
    return is_proper_list(v->data.pair.cdr);
}

// List predicate (list?)
static NadaValue *builtin_list_p(NadaValue *args, NadaEnv *env) {
    if (nada_is_nil(args) || !nada_is_nil(nada_cdr(args))) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "list? requires exactly 1 argument");
        return nada_create_bool(0);
    }

    NadaValue *val = nada_eval(nada_car(args), env);
    int result = is_proper_list(val);
    nada_free(val);
    return nada_create_bool(result);
}

// Atom predicate (atom?) - anything that's not a pair or nil
static NadaValue *builtin_atom_p(NadaValue *args, NadaEnv *env) {
    if (nada_is_nil(args) || !nada_is_nil(nada_cdr(args))) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "atom? requires exactly 1 argument");
        return nada_create_bool(0);
    }

    NadaValue *val = nada_eval(nada_car(args), env);
    int result = (val->type != NADA_PAIR && val->type != NADA_NIL);
    nada_free(val);
    return nada_create_bool(result);
}

// Logical negation (not)
static NadaValue *builtin_not(NadaValue *args, NadaEnv *env) {
    if (nada_is_nil(args) || !nada_is_nil(nada_cdr(args))) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "not requires exactly 1 argument");
        return nada_create_bool(0);
    }

    NadaValue *arg = nada_eval(nada_car(args), env);

    // In Lisp, only #f and nil are falsy, everything else is truthy
    int is_falsy = (arg->type == NADA_BOOL && arg->data.boolean == 0) ||
                   (arg->type == NADA_NIL);

    nada_free(arg);

    // Return the logical negation
    return nada_create_bool(is_falsy);
}

// sublist: Extract a portion of a list
static NadaValue *builtin_sublist(NadaValue *args, NadaEnv *env) {
    // Check for three arguments: list, start, end
    if (nada_is_nil(args) || nada_is_nil(nada_cdr(args)) ||
        nada_is_nil(nada_cdr(nada_cdr(args))) ||
        !nada_is_nil(nada_cdr(nada_cdr(nada_cdr(args))))) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT,
                          "sublist requires three arguments: list, start, end");
        return nada_create_nil();
    }

    // Evaluate arguments
    NadaValue *list_arg = nada_eval(nada_car(args), env);
    NadaValue *start_arg = nada_eval(nada_car(nada_cdr(args)), env);
    NadaValue *end_arg = nada_eval(nada_car(nada_cdr(nada_cdr(args))), env);

    // Validate types
    if (start_arg->type != NADA_NUM || end_arg->type != NADA_NUM) {
        nada_report_error(NADA_ERROR_TYPE_ERROR,
                          "sublist start and end must be numbers");
        nada_free(list_arg);
        nada_free(start_arg);
        nada_free(end_arg);
        return nada_create_nil();
    }

    // Convert to integers
    int start = nada_num_to_int(start_arg->data.number);
    int end = nada_num_to_int(end_arg->data.number);

    // Validate indices
    if (start < 0) {
        start = 0;  // Handle negative start gracefully
    }

    // Free the arguments once we've extracted the values we need
    nada_free(start_arg);
    nada_free(end_arg);

    // Handle empty list case
    if (nada_is_nil(list_arg)) {
        nada_free(list_arg);
        return nada_create_nil();
    }

    // Handle case where list isn't a proper list
    if (list_arg->type != NADA_PAIR) {
        nada_free(list_arg);
        return nada_create_nil();
    }

    // Extract the sublist
    NadaValue *current = list_arg;
    int pos = 0;

    // Skip elements before start
    while (pos < start && !nada_is_nil(current) && current->type == NADA_PAIR) {
        current = nada_cdr(current);
        pos++;
    }

    // Initialize result list
    NadaValue *items = nada_create_nil();

    // Collect elements from start to end
    while (pos < end && !nada_is_nil(current) && current->type == NADA_PAIR) {
        // Get the current element and make a deep copy
        NadaValue *element = nada_deep_copy(nada_car(current));
        
        // Create a new list with this element
        NadaValue *new_items = nada_cons(element, items);
        
        // Free temporary values
        nada_free(element);
        nada_free(items);
        
        // Update our items pointer
        items = new_items;
        
        // Move to next element
        current = nada_cdr(current);
        pos++;
    }

    // Reverse to get correct order
    NadaValue *result = nada_reverse(items);
    
    // Clean up
    nada_free(items);
    nada_free(list_arg);

    return result;
}

// list-ref: Get an element at a specific position in a list
static NadaValue *builtin_list_ref(NadaValue *args, NadaEnv *env) {
    // Check for exactly 2 arguments: list and index
    if (nada_is_nil(args) || nada_is_nil(nada_cdr(args)) ||
        !nada_is_nil(nada_cdr(nada_cdr(args)))) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT,
                          "list-ref requires exactly 2 arguments: list and index");
        return nada_create_nil();
    }

    // Evaluate the arguments
    NadaValue *list_arg = nada_eval(nada_car(args), env);
    NadaValue *index_arg = nada_eval(nada_car(nada_cdr(args)), env);

    // Check that index is a number
    if (index_arg->type != NADA_NUM) {
        nada_report_error(NADA_ERROR_TYPE_ERROR, "list-ref index must be a number");
        nada_free(list_arg);
        nada_free(index_arg);
        return nada_create_nil();
    }

    // Convert index to integer
    int index = nada_num_to_int(index_arg->data.number);
    if (index < 0) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "list-ref index must be non-negative");
        nada_free(list_arg);
        nada_free(index_arg);
        return nada_create_nil();
    }

    // Traverse the list to find the element
    NadaValue *current = list_arg;
    int current_pos = 0;

    while (current->type == NADA_PAIR && current_pos < index) {
        current = nada_cdr(current);
        current_pos++;
    }

    // Check if we found a valid element
    if (current->type != NADA_PAIR) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "list-ref index out of bounds");
        nada_free(list_arg);
        nada_free(index_arg);
        return nada_create_nil();
    }

    // Return a copy of the element
    NadaValue *result = nada_deep_copy(nada_car(current));

    // Clean up
    nada_free(list_arg);
    nada_free(index_arg);

    return result;
}

// Map function: Apply a function to each element of a list
static NadaValue *builtin_map(NadaValue *args, NadaEnv *env) {
    if (nada_is_nil(args) || nada_is_nil(nada_cdr(args))) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT,
                          "map requires at least 2 arguments: function and list");
        return nada_create_nil();
    }

    // Get the function (first argument)
    NadaValue *func_arg = nada_eval(nada_car(args), env);

    // Check that it's a function or a symbol that refers to a function
    if (func_arg->type != NADA_FUNC) {
        // If it's a symbol, try looking it up
        if (func_arg->type == NADA_SYMBOL) {
            NadaValue *func_val = nada_env_get(env, func_arg->data.symbol);
            nada_free(func_arg);
            func_arg = func_val;
            
            // Check again if it's a function
            if (func_arg->type != NADA_FUNC) {
                nada_report_error(NADA_ERROR_TYPE_ERROR, 
                                 "map requires a function as first argument, got: %s", 
                                 func_arg->data.symbol);
                nada_free(func_arg);
                return nada_create_nil();
            }
        } else {
            nada_report_error(NADA_ERROR_TYPE_ERROR, 
                             "map requires a function as first argument");
            nada_free(func_arg);
            return nada_create_nil();
        }
    }

    // Evaluate the list argument
    NadaValue *list_arg = nada_eval(nada_car(nada_cdr(args)), env);

    // Check that it's a list (or nil)
    if (list_arg->type != NADA_PAIR && list_arg->type != NADA_NIL) {
        nada_report_error(NADA_ERROR_TYPE_ERROR, "map requires a list as second argument");
        nada_free(func_arg);
        nada_free(list_arg);
        return nada_create_nil();
    }

    // Empty list case
    if (list_arg->type == NADA_NIL) {
        nada_free(func_arg);
        nada_free(list_arg);
        return nada_create_nil();
    }

    // Map the function over the list
    NadaValue *current = list_arg;
    NadaValue *mapped_items = nada_create_nil();

    while (current->type == NADA_PAIR) {
        // Get the current element - DO NOT EVALUATE IT YET!
        NadaValue *element = nada_car(current);
        NadaValue *mapped_value = NULL;

        if (func_arg->data.function.builtin) {
            // For built-in functions like 'car', we pass the element directly 
            // without evaluating it first
            
            // Check if the function is car, cdr, or similar list operations
            int is_list_op = 0;
            for (int i = 0; builtins[i].name != NULL; i++) {
                if (builtins[i].func == func_arg->data.function.builtin) {
                    const char *name = builtins[i].name;
                    if (strcmp(name, "car") == 0 || 
                        strcmp(name, "cdr") == 0 ||
                        strcmp(name, "cadr") == 0 ||
                        strcmp(name, "caddr") == 0 ||
                        strcmp(name, "list-ref") == 0) {
                        is_list_op = 1;
                        break;
                    }
                }
            }
            
            if (is_list_op) {
                // For list operations, create args without evaluating
                NadaValue *func_args = nada_cons(nada_deep_copy(element), nada_create_nil());
                
                // Apply the function directly
                mapped_value = func_arg->data.function.builtin(func_args, env);
                
                // Clean up
                nada_free(func_args);
            } else {
                // For other built-in functions, evaluate the element first
                NadaValue *eval_element = nada_eval(element, env);
                NadaValue *func_args = nada_cons(eval_element, nada_create_nil());
                
                // Apply the function
                mapped_value = func_arg->data.function.builtin(func_args, env);
                
                // Clean up
                nada_free(eval_element);
                nada_free(func_args);
            }
        } else {
            // For user-defined functions
            NadaValue *func_call = nada_cons(nada_deep_copy(element), nada_create_nil());
            mapped_value = apply_function(func_arg, func_call, env);
            nada_free(func_call);
        }

        // Add the result to our collected items (in reverse order for now)
        NadaValue *new_mapped_items = nada_cons(mapped_value, mapped_items);
        nada_free(mapped_value);
        nada_free(mapped_items);
        mapped_items = new_mapped_items;

        // Move to next element
        current = nada_cdr(current);
    }

    // Reverse the list to get correct order
    NadaValue *result = nada_reverse(mapped_items);
    
    // Free intermediate values
    nada_free(mapped_items);
    nada_free(func_arg);
    nada_free(list_arg);

    return result;
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

// Built-in function: undef
static NadaValue *builtin_undef(NadaValue *args, NadaEnv *env) {
    if (nada_is_nil(args) || !nada_is_nil(nada_cdr(args))) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "undef requires exactly one argument");
        return nada_create_bool(0);
    }

    // Get the symbol to undefine
    NadaValue *arg = nada_car(args);

    // Allow either quoted symbol or direct symbol
    if (arg->type == NADA_PAIR &&
        nada_car(arg)->type == NADA_SYMBOL &&
        strcmp(nada_car(arg)->data.symbol, "quote") == 0) {
        // It's a quoted symbol like 'x
        arg = nada_car(nada_cdr(arg));
    } else {
        // Evaluate to get the value
        arg = nada_eval(arg, env);
    }

    if (arg->type != NADA_SYMBOL) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "undef requires a symbol argument");
        nada_free(arg);
        return nada_create_bool(0);
    }

    // Remove the binding
    nada_env_remove(env, arg->data.symbol);

    nada_free(arg);
    return nada_create_bool(1);  // Return true for success
}

// Cons function: Create a pair
static NadaValue *builtin_cons(NadaValue *args, NadaEnv *env) {
    // Check for exactly 2 arguments
    if (nada_is_nil(args) || nada_is_nil(nada_cdr(args)) ||
        !nada_is_nil(nada_cdr(nada_cdr(args)))) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "cons requires exactly 2 arguments");
        return nada_create_nil();
    }

    // Evaluate both arguments
    NadaValue *car_val = nada_eval(nada_car(args), env);
    NadaValue *cdr_val = nada_eval(nada_car(nada_cdr(args)), env);

    // Create a new pair
    NadaValue *result = nada_cons(car_val, cdr_val);

    // Free the evaluated arguments since nada_cons makes copies
    nada_free(car_val);
    nada_free(cdr_val);

    return result;
}

// Fix in NadaEval.c
static NadaValue *builtin_list(NadaValue *args, NadaEnv *env) {
    // Start with an empty list
    NadaValue *result = nada_create_nil();

    // Process arguments in reverse (last to first)
    NadaValue *arg_ptr = args;
    NadaValue **args_array = NULL;
    int count = 0;

    // First, count the arguments
    while (!nada_is_nil(arg_ptr)) {
        count++;
        arg_ptr = nada_cdr(arg_ptr);
    }

    // Allocate array to store evaluated arguments
    args_array = malloc(count * sizeof(NadaValue *));

    // Evaluate all arguments
    arg_ptr = args;
    for (int i = 0; i < count; i++) {
        args_array[i] = nada_eval(nada_car(arg_ptr), env);
        arg_ptr = nada_cdr(arg_ptr);
    }

    // Build list from end to beginning
    for (int i = count - 1; i >= 0; i--) {
        NadaValue *new_result = nada_cons(args_array[i], result);
        nada_free(result);
        result = new_result;
    }

    // Free the evaluated argument values
    for (int i = 0; i < count; i++) {
        nada_free(args_array[i]);
    }
    free(args_array);

    return result;
}

// Add this function:

static NadaValue *builtin_if(NadaValue *args, NadaEnv *env) {
    // if requires at least 2 arguments (condition, then-expr, [else-expr])
    if (nada_is_nil(args) || nada_is_nil(nada_cdr(args))) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "if requires at least 2 arguments");
        return nada_create_nil();
    }

    // Evaluate the condition
    NadaValue *condition = nada_eval(nada_car(args), env);

    // Check if condition is true (anything other than #f or nil)
    int is_true = !(condition->type == NADA_BOOL && condition->data.boolean == 0) &&
                  !(condition->type == NADA_NIL);

    nada_free(condition);

    if (is_true) {
        // Evaluate the then-expression
        return nada_eval(nada_car(nada_cdr(args)), env);
    } else {
        // Check if we have an else expression
        NadaValue *else_part = nada_cdr(nada_cdr(args));
        if (nada_is_nil(else_part)) {
            return nada_create_nil();  // No else clause, return nil
        }
        // Evaluate the else-expression
        return nada_eval(nada_car(else_part), env);
    }
}

// Length function - count elements in a list
static NadaValue *builtin_length(NadaValue *args, NadaEnv *env) {
    if (nada_is_nil(args) || !nada_is_nil(nada_cdr(args))) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "length requires exactly 1 argument");
        return nada_create_num_from_int(0);
    }

    NadaValue *list_val = nada_eval(nada_car(args), env);

    // For nil (empty list), return 0
    if (nada_is_nil(list_val)) {
        nada_free(list_val);
        return nada_create_num_from_int(0);
    }

    // For non-list values, return error
    if (list_val->type != NADA_PAIR) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "length requires a list argument");
        nada_free(list_val);
        return nada_create_num_from_int(0);
    }

    // Count elements
    int count = 0;
    NadaValue *current = list_val;

    while (current->type == NADA_PAIR) {
        count++;
        current = current->data.pair.cdr;
    }

    nada_free(list_val);
    return nada_create_num_from_int(count);
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

// Built-in special form: begin
static NadaValue *builtin_begin(NadaValue *args, NadaEnv *env) {
    // Handle empty begin
    if (nada_is_nil(args)) {
        return nada_create_nil();
    }

    // Evaluate each expression in sequence, returning the last result
    NadaValue *result = nada_create_nil();
    NadaValue *expr = args;

    while (!nada_is_nil(expr)) {
        // Always free the previous result
        nada_free(result);

        // Evaluate the next expression
        result = nada_eval(nada_car(expr), env);
        expr = nada_cdr(expr);
    }

    return result;
}

// Built-in special form: or
static NadaValue *builtin_or(NadaValue *args, NadaEnv *env) {
    // Empty 'or' evaluates to #f
    if (nada_is_nil(args)) {
        return nada_create_bool(0);
    }

    // Evaluate each argument in sequence
    NadaValue *result = nada_create_bool(0);
    NadaValue *expr = args;

    while (!nada_is_nil(expr)) {
        // Free the previous result
        nada_free(result);

        // Evaluate the current expression
        result = nada_eval(nada_car(expr), env);

        // If truthy (anything except #f or nil), short-circuit and return it
        if (!(result->type == NADA_BOOL && result->data.boolean == 0) &&
            !(result->type == NADA_NIL)) {
            return result;
        }

        // Otherwise, move to next argument
        expr = nada_cdr(expr);
    }

    // All arguments were falsy, return the last result (which is falsy)
    return result;
}

// Built-in special form: and
static NadaValue *builtin_and(NadaValue *args, NadaEnv *env) {
    // Empty 'and' evaluates to #t
    if (nada_is_nil(args)) {
        return nada_create_bool(1);
    }

    // Evaluate each argument in sequence
    NadaValue *result = nada_create_bool(1);
    NadaValue *expr = args;

    while (!nada_is_nil(expr)) {
        // Free the previous result
        nada_free(result);

        // Evaluate the current expression
        result = nada_eval(nada_car(expr), env);

        // If falsy (#f or nil), short-circuit and return it
        if ((result->type == NADA_BOOL && result->data.boolean == 0) ||
            (result->type == NADA_NIL)) {
            return result;
        }

        // Otherwise, move to next argument
        expr = nada_cdr(expr);
    }

    // All arguments were truthy, return the last result (which is truthy)
    return result;
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
static BuiltinFunc get_builtin_func(const char *name) {
    for (int i = 0; builtins[i].name != NULL; i++) {
        if (strcmp(name, builtins[i].name) == 0) {
            return builtins[i].func;
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
        return nada_env_get(env, expr->data.symbol);
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

            // Regular function application
            BuiltinFunc func = get_builtin_func(op->data.symbol);
            if (func != NULL) {
                return func(args, env);
            }

            // Try to apply as a user-defined function
            NadaValue *func_val = nada_env_get(env, op->data.symbol);
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
