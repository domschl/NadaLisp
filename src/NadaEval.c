#include "NadaEval.h"
#include "NadaParser.h"
#include "NadaString.h"  // Add this include
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

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
    fprintf(stderr, "Error: symbol '%s' not found in environment\n", name);
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
        fprintf(stderr, "Error: quote requires exactly 1 argument\n");
        return nada_create_nil();
    }

    // Return unevaluated argument (make a deep copy to prevent modification)
    return nada_deep_copy(nada_car(args));
}

// Built-in function: car
static NadaValue *builtin_car(NadaValue *args, NadaEnv *env) {
    if (nada_is_nil(args) || !nada_is_nil(nada_cdr(args))) {
        fprintf(stderr, "Error: car requires exactly 1 argument\n");
        return nada_create_nil();
    }

    // Evaluate the argument
    NadaValue *val = nada_eval(nada_car(args), env);

    // Check that it's a pair
    if (val->type != NADA_PAIR) {
        fprintf(stderr, "Error: car called on non-pair\n");
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
        fprintf(stderr, "Error: cdr takes exactly one argument\n");
        return nada_create_nil();
    }

    NadaValue *arg = nada_eval(nada_car(args), env);
    if (arg->type != NADA_PAIR) {
        fprintf(stderr, "Error: cdr requires a list argument\n");
        nada_free(arg);
        return nada_create_nil();
    }

    // Get the cdr and make a deep copy of it
    NadaValue *result = nada_deep_copy(nada_cdr(arg));

    // Free the evaluated argument
    nada_free(arg);

    return result;
}

// Built-in function: add
static NadaValue *builtin_add(NadaValue *args, NadaEnv *env) {
    int result = 0;
    NadaValue *current = args;

    while (!nada_is_nil(current)) {
        NadaValue *arg = nada_eval(nada_car(current), env);
        if (arg->type != NADA_INT) {
            fprintf(stderr, "Error: '+' requires integer arguments\n");
            nada_free(arg);
            return nada_create_int(0);
        }
        result += arg->data.integer;
        nada_free(arg);
        current = nada_cdr(current);
    }

    return nada_create_int(result);
}

// Built-in function: subtract
static NadaValue *builtin_subtract(NadaValue *args, NadaEnv *env) {
    if (nada_is_nil(args)) {
        fprintf(stderr, "Error: '-' requires at least one argument\n");
        return nada_create_int(0);
    }

    NadaValue *first = nada_eval(nada_car(args), env);
    if (first->type != NADA_INT) {
        fprintf(stderr, "Error: '-' requires integer arguments\n");
        nada_free(first);
        return nada_create_int(0);
    }

    int result = first->data.integer;
    nada_free(first);

    NadaValue *rest = nada_cdr(args);
    if (nada_is_nil(rest)) {
        // Unary minus
        return nada_create_int(-result);
    }

    // Binary subtraction
    while (!nada_is_nil(rest)) {
        NadaValue *arg = nada_eval(nada_car(rest), env);
        if (arg->type != NADA_INT) {
            fprintf(stderr, "Error: '-' requires integer arguments\n");
            nada_free(arg);
            return nada_create_int(0);
        }
        result -= arg->data.integer;
        nada_free(arg);
        rest = nada_cdr(rest);
    }

    return nada_create_int(result);
}

// Built-in function: multiply
static NadaValue *builtin_multiply(NadaValue *args, NadaEnv *env) {
    int result = 1;
    NadaValue *current = args;

    while (!nada_is_nil(current)) {
        NadaValue *arg = nada_eval(nada_car(current), env);
        if (arg->type != NADA_INT) {
            fprintf(stderr, "Error: '*' requires integer arguments\n");
            nada_free(arg);
            return nada_create_int(0);
        }
        result *= arg->data.integer;
        nada_free(arg);
        current = nada_cdr(current);
    }

    return nada_create_int(result);
}

// Built-in function: divide
static NadaValue *builtin_divide(NadaValue *args, NadaEnv *env) {
    if (nada_is_nil(args)) {
        fprintf(stderr, "Error: '/' requires at least one argument\n");
        return nada_create_int(0);
    }

    NadaValue *first = nada_eval(nada_car(args), env);
    if (first->type != NADA_INT) {
        fprintf(stderr, "Error: '/' requires integer arguments\n");
        nada_free(first);
        return nada_create_int(0);
    }

    int result = first->data.integer;
    nada_free(first);

    NadaValue *rest = nada_cdr(args);
    if (nada_is_nil(rest)) {
        // Unary division (1/x)
        if (result == 0) {
            fprintf(stderr, "Error: division by zero\n");
            return nada_create_int(0);
        }
        return nada_create_int(1 / result);
    }

    // Binary division
    while (!nada_is_nil(rest)) {
        NadaValue *arg = nada_eval(nada_car(rest), env);
        if (arg->type != NADA_INT) {
            fprintf(stderr, "Error: '/' requires integer arguments\n");
            nada_free(arg);
            return nada_create_int(0);
        }

        if (arg->data.integer == 0) {
            fprintf(stderr, "Error: division by zero\n");
            nada_free(arg);
            return nada_create_int(0);
        }

        result /= arg->data.integer;
        nada_free(arg);
        rest = nada_cdr(rest);
    }

    return nada_create_int(result);
}

// Built-in special form: define
static NadaValue *builtin_define(NadaValue *args, NadaEnv *env) {
    // Check that we have at least two arguments
    if (nada_is_nil(args) || nada_is_nil(nada_cdr(args))) {
        fprintf(stderr, "Error: define requires at least 2 arguments\n");
        return nada_create_nil();
    }

    NadaValue *first = nada_car(args);

    // Case 1: Define a variable - (define symbol expr)
    if (first->type == NADA_SYMBOL) {
        // This is the existing implementation
        NadaValue *val_expr = nada_car(nada_cdr(args));
        NadaValue *val = nada_eval(val_expr, env);
        nada_env_set(env, first->data.symbol, val);
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

        // Return the function name
        return nada_create_symbol(func_name->data.symbol);
    }

    fprintf(stderr, "Error: invalid define syntax\n");
    return nada_create_nil();
}

// Built-in special form: lambda
static NadaValue *builtin_lambda(NadaValue *args, NadaEnv *env) {
    // Check that we have at least two arguments: params and body
    if (nada_is_nil(args) || nada_is_nil(nada_cdr(args))) {
        fprintf(stderr, "Error: lambda requires parameters and body\n");
        return nada_create_nil();
    }

    // First argument must be a list of symbols (parameter names)
    NadaValue *params = nada_car(args);
    // Validate parameters (should all be symbols)
    NadaValue *param_check = params;
    while (param_check->type == NADA_PAIR) {
        if (nada_car(param_check)->type != NADA_SYMBOL) {
            fprintf(stderr, "Error: lambda parameters must be symbols\n");
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

        // Each clause should be a list
        if (clause->type != NADA_PAIR) {
            fprintf(stderr, "Error: cond clause must be a list\n");
            return nada_create_nil();
        }

        // Get and evaluate the condition
        NadaValue *condition = nada_car(clause);
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
                // Free previous intermediate result
                if (result->type != NADA_NIL) {
                    nada_free(result);
                }

                // Evaluate next expression
                result = nada_eval(nada_car(body), env);
                body = nada_cdr(body);
            }

            return result;
        }

        // Move to next clause
        clauses = nada_cdr(clauses);
    }

    // No condition matched
    return nada_create_nil();
}

// Built-in special form: let
static NadaValue *builtin_let(NadaValue *args, NadaEnv *env) {
    // Check for at least one argument (bindings list)
    if (nada_is_nil(args)) {
        fprintf(stderr, "Error: let requires bindings and body\n");
        return nada_create_nil();
    }

    // First argument must be a list of bindings
    NadaValue *bindings = nada_car(args);
    if (bindings->type != NADA_PAIR && bindings->type != NADA_NIL) {
        fprintf(stderr, "Error: let bindings must be a list\n");
        return nada_create_nil();
    }

    // Body is the rest of the args
    NadaValue *body = nada_cdr(args);
    if (nada_is_nil(body)) {
        fprintf(stderr, "Error: let requires a body\n");
        return nada_create_nil();
    }

    // Create a new environment with the current environment as parent
    NadaEnv *let_env = nada_env_create(env);

    // Process each binding
    NadaValue *binding_list = bindings;
    while (!nada_is_nil(binding_list)) {
        NadaValue *binding = nada_car(binding_list);

        // Each binding should be a list (var val)
        if (binding->type != NADA_PAIR) {
            fprintf(stderr, "Error: each let binding must be a list\n");
            nada_env_free(let_env);
            return nada_create_nil();
        }

        // Extract variable name (first element)
        NadaValue *var = nada_car(binding);
        if (var->type != NADA_SYMBOL) {
            fprintf(stderr, "Error: binding variable must be a symbol\n");
            nada_env_free(let_env);
            return nada_create_nil();
        }

        // Extract and evaluate the value (second element)
        NadaValue *val_expr = nada_car(nada_cdr(binding));
        if (val_expr == NULL) {
            fprintf(stderr, "Error: binding requires a value\n");
            nada_env_free(let_env);
            return nada_create_nil();
        }

        // Evaluate in the original environment, not the new one
        NadaValue *val = nada_eval(val_expr, env);

        // Bind the variable in the new environment
        nada_env_set(let_env, var->data.symbol, val);

        // Move to the next binding
        binding_list = nada_cdr(binding_list);
    }

    // Evaluate the body expressions in the new environment, keeping only the last result
    NadaValue *result = nada_create_nil();
    NadaValue *current_body = body;

    while (!nada_is_nil(current_body)) {
        // Free the previous result (unless it's the first iteration)
        if (result->type != NADA_NIL) {
            nada_free(result);
        }

        // Evaluate the next expression
        result = nada_eval(nada_car(current_body), let_env);

        // Move to the next expression
        current_body = nada_cdr(current_body);
    }

    // Clean up the let environment
    nada_env_free(let_env);

    return result;
}

// Apply a function to arguments
static NadaValue *apply_function(NadaValue *func, NadaValue *args, NadaEnv *env) {
    if (func->type != NADA_FUNC) {
        fprintf(stderr, "Error: attempt to apply non-function\n");
        return nada_create_nil();
    }

    // Handle built-in functions
    if (func->data.function.builtin != NULL) {
        // Call the built-in function directly
        return func->data.function.builtin(args, env);
    }

    // Original code for user-defined functions...
    // ...

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

        // Move to next parameter and argument
        param = nada_cdr(param);
        arg = nada_cdr(arg);
    }

    // Check for argument count mismatches
    if (!nada_is_nil(param)) {
        fprintf(stderr, "Error: too few arguments\n");
        nada_env_free(func_env);
        return nada_create_nil();
    }

    if (!nada_is_nil(arg)) {
        fprintf(stderr, "Error: too many arguments\n");
        nada_env_free(func_env);
        return nada_create_nil();
    }

    // Evaluate body expressions in sequence, returning the last result
    NadaValue *result = nada_create_nil();
    NadaValue *body = func->data.function.body;

    while (!nada_is_nil(body)) {
        // Free previous result except for the first iteration
        if (result != NULL && result->type != NADA_NIL) {
            nada_free(result);
        }

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
        fprintf(stderr, "Error: < requires exactly 2 arguments\n");
        return nada_create_bool(0);
    }

    NadaValue *first = nada_eval(nada_car(args), env);
    NadaValue *second = nada_eval(nada_car(nada_cdr(args)), env);

    if (first->type != NADA_INT || second->type != NADA_INT) {
        fprintf(stderr, "Error: < requires integer arguments\n");
        nada_free(first);
        nada_free(second);
        return nada_create_bool(0);
    }

    int result = first->data.integer < second->data.integer;

    nada_free(first);
    nada_free(second);

    return nada_create_bool(result);
}

// Less than or equal (<=)
static NadaValue *builtin_less_equal(NadaValue *args, NadaEnv *env) {
    // Similar to less_than but with <= operator
    if (nada_is_nil(args) || nada_is_nil(nada_cdr(args)) ||
        !nada_is_nil(nada_cdr(nada_cdr(args)))) {
        fprintf(stderr, "Error: <= requires exactly 2 arguments\n");
        return nada_create_bool(0);
    }

    NadaValue *first = nada_eval(nada_car(args), env);
    NadaValue *second = nada_eval(nada_car(nada_cdr(args)), env);

    if (first->type != NADA_INT || second->type != NADA_INT) {
        fprintf(stderr, "Error: <= requires integer arguments\n");
        nada_free(first);
        nada_free(second);
        return nada_create_bool(0);
    }

    int result = first->data.integer <= second->data.integer;

    nada_free(first);
    nada_free(second);

    return nada_create_bool(result);
}

// Greater than (>)
static NadaValue *builtin_greater_than(NadaValue *args, NadaEnv *env) {
    // Similar to less_than but with > operator
    if (nada_is_nil(args) || nada_is_nil(nada_cdr(args)) ||
        !nada_is_nil(nada_cdr(nada_cdr(args)))) {
        fprintf(stderr, "Error: > requires exactly 2 arguments\n");
        return nada_create_bool(0);
    }

    NadaValue *first = nada_eval(nada_car(args), env);
    NadaValue *second = nada_eval(nada_car(nada_cdr(args)), env);

    if (first->type != NADA_INT || second->type != NADA_INT) {
        fprintf(stderr, "Error: > requires integer arguments\n");
        nada_free(first);
        nada_free(second);
        return nada_create_bool(0);
    }

    int result = first->data.integer > second->data.integer;

    nada_free(first);
    nada_free(second);

    return nada_create_bool(result);
}

// Greater than or equal (>=)
static NadaValue *builtin_greater_equal(NadaValue *args, NadaEnv *env) {
    // Similar to less_than but with >= operator
    if (nada_is_nil(args) || nada_is_nil(nada_cdr(args)) ||
        !nada_is_nil(nada_cdr(nada_cdr(args)))) {
        fprintf(stderr, "Error: >= requires exactly 2 arguments\n");
        return nada_create_bool(0);
    }

    NadaValue *first = nada_eval(nada_car(args), env);
    NadaValue *second = nada_eval(nada_car(nada_cdr(args)), env);

    if (first->type != NADA_INT || second->type != NADA_INT) {
        fprintf(stderr, "Error: >= requires integer arguments\n");
        nada_free(first);
        nada_free(second);
        return nada_create_bool(0);
    }

    int result = first->data.integer >= second->data.integer;

    nada_free(first);
    nada_free(second);

    return nada_create_bool(result);
}

// Numeric equality (=)
static NadaValue *builtin_numeric_equal(NadaValue *args, NadaEnv *env) {
    if (nada_is_nil(args) || nada_is_nil(nada_cdr(args)) ||
        !nada_is_nil(nada_cdr(nada_cdr(args)))) {
        fprintf(stderr, "Error: = requires exactly 2 arguments\n");
        return nada_create_bool(0);
    }

    NadaValue *first = nada_eval(nada_car(args), env);
    NadaValue *second = nada_eval(nada_car(nada_cdr(args)), env);

    if (first->type != NADA_INT || second->type != NADA_INT) {
        fprintf(stderr, "Error: = requires integer arguments\n");
        nada_free(first);
        nada_free(second);
        return nada_create_bool(0);
    }

    int result = first->data.integer == second->data.integer;

    nada_free(first);
    nada_free(second);

    return nada_create_bool(result);
}

// Identity equality (eq?)
static NadaValue *builtin_eq(NadaValue *args, NadaEnv *env) {
    if (nada_is_nil(args) || nada_is_nil(nada_cdr(args)) ||
        !nada_is_nil(nada_cdr(nada_cdr(args)))) {
        fprintf(stderr, "Error: eq? requires exactly 2 arguments\n");
        return nada_create_bool(0);
    }

    NadaValue *first = nada_eval(nada_car(args), env);
    NadaValue *second = nada_eval(nada_car(nada_cdr(args)), env);

    int result = 0;

    // Must be the same type to be eq?
    if (first->type == second->type) {
        switch (first->type) {
        case NADA_INT:
            result = (first->data.integer == second->data.integer);
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
    case NADA_INT:
        return a->data.integer == b->data.integer;
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
        fprintf(stderr, "Error: equal? requires exactly 2 arguments\n");
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
        fprintf(stderr, "Error: null? requires exactly 1 argument\n");
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
        fprintf(stderr, "Error: integer? requires exactly 1 argument\n");
        return nada_create_bool(0);
    }

    NadaValue *val = nada_eval(nada_car(args), env);
    int result = (val->type == NADA_INT);
    nada_free(val);
    return nada_create_bool(result);
}

// String predicate (string?)
static NadaValue *builtin_string_p(NadaValue *args, NadaEnv *env) {
    if (nada_is_nil(args) || !nada_is_nil(nada_cdr(args))) {
        fprintf(stderr, "Error: string? requires exactly 1 argument\n");
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
        fprintf(stderr, "Error: symbol? requires exactly 1 argument\n");
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
        fprintf(stderr, "Error: boolean? requires exactly 1 argument\n");
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
        fprintf(stderr, "Error: pair? requires exactly 1 argument\n");
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
        fprintf(stderr, "Error: function? requires exactly 1 argument\n");
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
        fprintf(stderr, "Error: list? requires exactly 1 argument\n");
        return nada_create_bool(0);
    }

    NadaValue *val = nada_eval(nada_car(args), env);
    int result = is_proper_list(val);
    nada_free(val);
    return nada_create_bool(result);
}

// Atom predicate (atom?) - anything that's not a pair
static NadaValue *builtin_atom_p(NadaValue *args, NadaEnv *env) {
    if (nada_is_nil(args) || !nada_is_nil(nada_cdr(args))) {
        fprintf(stderr, "Error: atom? requires exactly 1 argument\n");
        return nada_create_bool(0);
    }

    NadaValue *val = nada_eval(nada_car(args), env);
    int result = (val->type != NADA_PAIR);
    nada_free(val);
    return nada_create_bool(result);
}

// Type to represent a built-in function
typedef NadaValue *(*BuiltinFunc)(NadaValue *, NadaEnv *);

// Structure to hold built-in function info
typedef struct {
    const char *name;
    BuiltinFunc func;
} BuiltinFuncInfo;

// Forward declaration of the builtins array
static BuiltinFuncInfo builtins[];

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
        fprintf(stderr, "Error: builtin? requires exactly 1 argument\n");
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
        // Add symbol to the front of our list
        *list = nada_cons(nada_create_symbol(binding->name), *list);
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
        fprintf(stderr, "Error: env-symbols takes no arguments\n");
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
        case NADA_INT:
            printf("Integer (%d)\n", binding->value->data.integer);
            break;
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
        fprintf(stderr, "Error: env-describe takes no arguments\n");
        return nada_create_nil();
    }

    printf("Environment contents:\n");

    print_bindings(env, 0);

    return nada_create_nil();  // Just return nil
}

// Helper to serialize a value to the file
void serialize_value(NadaValue *val, FILE *f) {
    switch (val->type) {
    case NADA_INT:
        fprintf(f, "%d", val->data.integer);
        break;
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
        fprintf(stderr, "Error: save-environment requires exactly one filename argument\n");
        return nada_create_bool(0);
    }

    NadaValue *filename_arg = nada_eval(nada_car(args), env);
    if (filename_arg->type != NADA_STRING) {
        fprintf(stderr, "Error: save-environment requires a string filename\n");
        nada_free(filename_arg);
        return nada_create_bool(0);
    }

    FILE *file = fopen(filename_arg->data.string, "w");
    if (!file) {
        fprintf(stderr, "Error: could not open file %s for writing\n",
                filename_arg->data.string);
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
        fprintf(stderr, "Error: load-file requires exactly one filename argument\n");
        return nada_create_bool(0);
    }

    NadaValue *filename_arg = nada_eval(nada_car(args), env);
    if (filename_arg->type != NADA_STRING) {
        fprintf(stderr, "Error: load-file requires a string filename\n");
        nada_free(filename_arg);
        return nada_create_bool(0);
    }

    FILE *file = fopen(filename_arg->data.string, "r");
    if (!file) {
        fprintf(stderr, "Error: could not open file %s for reading\n",
                filename_arg->data.string);
        nada_free(filename_arg);
        return nada_create_bool(0);
    }

    // Read and evaluate the file line by line
    char buffer[1024];
    NadaValue *result = nada_create_nil();

    while (fgets(buffer, sizeof(buffer), file)) {
        // Free previous result
        if (result != NULL) {
            nada_free(result);
        }

        // Parse and evaluate the expression
        NadaValue *expr = nada_parse(buffer);
        result = nada_eval(expr, env);
        nada_free(expr);
    }

    fclose(file);
    nada_free(filename_arg);

    return result;  // Return the result of the last evaluation
}

// Built-in function: undef
static NadaValue *builtin_undef(NadaValue *args, NadaEnv *env) {
    if (nada_is_nil(args) || !nada_is_nil(nada_cdr(args))) {
        fprintf(stderr, "Error: undef requires exactly one argument\n");
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
        fprintf(stderr, "Error: undef requires a symbol argument\n");
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
        fprintf(stderr, "Error: cons requires exactly 2 arguments\n");
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

// Add to the builtins table (keep all string functions here)
static BuiltinFuncInfo builtins[] = {
    {"quote", builtin_quote},
    {"car", builtin_car},
    {"cdr", builtin_cdr},
    {"+", builtin_add},
    {"-", builtin_subtract},
    {"*", builtin_multiply},
    {"/", builtin_divide},
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
    {"read-from-string", builtin_read_from_string},
    {"write-to-string", builtin_write_to_string},

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
        // Each of these creates values that should be tracked
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
    // Self-evaluating expressions: integers, strings, booleans, and nil
    if (expr->type == NADA_INT || expr->type == NADA_STRING ||
        expr->type == NADA_BOOL || expr->type == NADA_NIL) {
        // Make a copy to avoid double-freeing
        switch (expr->type) {
        case NADA_INT:
            return nada_create_int(expr->data.integer);
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
        fprintf(stderr, "Error: not a function\n");
        return nada_create_nil();
    }

    // Default case
    fprintf(stderr, "Error: cannot evaluate expression\n");
    return nada_create_nil();
}

// In the initialization function (or main), add:
void nada_init(void) {
}

// Add to NadaEval.c
NadaValue *nada_create_builtin_function(NadaValue *(*func)(NadaValue *, NadaEnv *)) {
    NadaValue *val = malloc(sizeof(NadaValue));
    nada_increment_allocations();  // Use the function instead of direct access
    val->type = NADA_FUNC;
    val->data.function.params = NULL;   // No explicit parameters for builtins
    val->data.function.body = NULL;     // No body for builtins
    val->data.function.env = NULL;      // No closure environment
    val->data.function.builtin = func;  // Store the function pointer
    return val;
}
