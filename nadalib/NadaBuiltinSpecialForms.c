#include <stdio.h>
#include <string.h>

#include "NadaEval.h"
#include "NadaError.h"
#include "NadaBuiltinSpecialForms.h"

// Built-in function: quote
NadaValue *builtin_quote(NadaValue *args, NadaEnv *env) {
    // Check for exactly 1 argument
    if (nada_is_nil(args) || !nada_is_nil(nada_cdr(args))) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "quote requires exactly 1 argument");
        return nada_create_nil();
    }

    // Return unevaluated argument (make a deep copy to prevent modification)
    return nada_deep_copy(nada_car(args));
}

// Built-in special form: define
NadaValue *builtin_define(NadaValue *args, NadaEnv *env) {
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

        // Free the original function value *after* it's been copied and stored.
        // Temporarily NULL the env pointer to prevent premature release of the
        // environment, which is now correctly referenced by the copy in the binding.
        if (func->type == NADA_FUNC) {  // Safety check
            // We don't need to save/restore, just prevent release during this free
            func->data.function.env = NULL;
        }
        nada_free(func);  // Free the original func value

        // Return the function name
        return nada_create_symbol(func_name->data.symbol);
    }

    nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "invalid define syntax");
    return nada_create_nil();
}

// Built-in function: undef
NadaValue *builtin_undef(NadaValue *args, NadaEnv *env) {
    if (nada_is_nil(args) || !nada_is_nil(nada_cdr(args))) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "undef requires exactly one argument");
        return nada_create_bool(0);
    }

    // Get the symbol to undefine
    NadaValue *arg = nada_car(args);
    NadaValue *to_free = NULL;

    // Allow either quoted symbol or direct symbol
    if (arg->type == NADA_PAIR &&
        nada_car(arg)->type == NADA_SYMBOL &&
        strcmp(nada_car(arg)->data.symbol, "quote") == 0) {
        // It's a quoted symbol like 'x - DON'T free this later!
        arg = nada_car(nada_cdr(arg));
    } else {
        // Evaluate to get the value - this needs to be freed
        to_free = nada_eval(arg, env);
        arg = to_free;
    }

    if (arg->type != NADA_SYMBOL) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "undef requires a symbol argument");
        if (to_free) nada_free(to_free);
        return nada_create_bool(0);
    }

    // Remove the binding
    nada_env_remove(env, arg->data.symbol);

    // Only free if we allocated a new value
    if (to_free) nada_free(to_free);

    return nada_create_bool(1);  // Return true for success
}

// Built-in special form: lambda
NadaValue *builtin_lambda(NadaValue *args, NadaEnv *env) {
    // Check that we have at least two arguments: params and body
    if (nada_is_nil(args) || nada_is_nil(nada_cdr(args))) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "lambda requires parameters and body");
        return nada_create_nil();
    }

    // First argument must be either a list of symbols or a single symbol
    NadaValue *params = nada_car(args);

    // Case 1: Single symbol for variadic function (lambda args body)
    if (params->type == NADA_SYMBOL) {
        // This is a valid case - all args will be collected as a list
    }
    // Case 2: List of parameters
    else if (params->type == NADA_PAIR || params->type == NADA_NIL) {
        // Validate parameters (should all be symbols)
        NadaValue *param_check = params;
        while (param_check->type == NADA_PAIR) {
            if (nada_car(param_check)->type != NADA_SYMBOL) {
                nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "lambda parameters must be symbols");
                return nada_create_nil();
            }
            param_check = nada_cdr(param_check);
        }

        // TODO: Check for dotted pair notation for rest params: (a b . rest)
    } else {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "lambda parameters must be a symbol or list");
        return nada_create_nil();
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

NadaValue *builtin_if(NadaValue *args, NadaEnv *env) {
    // if requires at least 2 arguments (condition, then-expr, [else-expr])
    if (nada_is_nil(args) || nada_is_nil(nada_cdr(args))) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "if requires at least 2 arguments");
        return nada_create_nil();
    }

    // Evaluate the condition
    NadaValue *condition = nada_eval(nada_car(args), env);

    // Check if condition is true (only #f is falsy in standard Scheme)
    int is_true = !(condition->type == NADA_BOOL && condition->data.boolean == 0);

    nada_free(condition);

    if (is_true) {
        // Evaluate and return a deep copy of the then-expression
        NadaValue *then_result = nada_eval(nada_car(nada_cdr(args)), env);
        NadaValue *result_copy = nada_deep_copy(then_result);
        nada_free(then_result);
        return result_copy;
    } else {
        // Check if we have an else expression
        NadaValue *else_part = nada_cdr(nada_cdr(args));
        if (nada_is_nil(else_part)) {
            return nada_create_nil();  // No else clause, return nil
        }
        // Evaluate the else-expression
        NadaValue *else_result = nada_eval(nada_car(else_part), env);
        NadaValue *result_copy = nada_deep_copy(else_result);
        nada_free(else_result);
        return result_copy;
    }
}

// Built-in special form: cond
NadaValue *builtin_cond(NadaValue *args, NadaEnv *env) {
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
                nada_free(result);

                // Get a deep copy of the evaluated expression
                result = nada_eval(nada_car(body), env);
                if (nada_is_nil(nada_cdr(body))) {
                    // If this is the last body expression, return a deep copy
                    NadaValue *result_copy = nada_deep_copy(result);
                    nada_free(result);
                    return result_copy;
                }
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
                nada_free(result);

                // Get a deep copy of the evaluated expression
                result = nada_eval(nada_car(body), env);
                if (nada_is_nil(nada_cdr(body))) {
                    // If this is the last body expression, return a deep copy
                    NadaValue *result_copy = nada_deep_copy(result);
                    nada_free(result);
                    return result_copy;
                }
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
NadaValue *builtin_let(NadaValue *args, NadaEnv *env) {
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
        const char *loop_name = first_arg->data.symbol;
        NadaValue *bindings = nada_car(nada_cdr(args));
        NadaValue *body = nada_cdr(nada_cdr(args));

        // Create the loop environment with an extra reference
        NadaEnv *loop_env = nada_env_create(env);
        nada_env_add_ref(loop_env);  // Add 'extra' scope reference to keep it alive

        // Evaluate initial binding values in the original environment
        NadaValue *current_binding = bindings;
        while (!nada_is_nil(current_binding)) {
            NadaValue *binding = nada_car(current_binding);
            if (binding->type != NADA_PAIR ||
                nada_car(binding)->type != NADA_SYMBOL) {
                nada_report_error(NADA_ERROR_INVALID_ARGUMENT,
                                  "named let binding must be (var value)");
                nada_env_release(loop_env);
                nada_env_release(loop_env);  // 'extra' scope reference
                return nada_create_nil();
            }
            const char *var_name = nada_car(binding)->data.symbol;
            NadaValue *val_expr = nada_car(nada_cdr(binding));
            NadaValue *val = nada_eval(val_expr, env);
            nada_env_set(loop_env, var_name, val);
            if (val->type == NADA_ERROR) {   // Check for eval errors
                nada_env_release(loop_env);  // Release extra scope ref
                nada_env_release(loop_env);  // Release initial ref
                return val;                  // Propagate error
            }
            nada_free(val);
            current_binding = nada_cdr(current_binding);
        }

        // Build the parameter list for the loop function from binding names
        NadaValue *params = nada_create_nil();
        current_binding = bindings;  // Reset to start of bindings
        while (!nada_is_nil(current_binding)) {
            NadaValue *binding = nada_car(current_binding);
            NadaValue *param_symbol = nada_car(binding);
            // Need deep copy here as original symbols are part of bindings list
            NadaValue *param_symbol_copy = nada_deep_copy(param_symbol);
            NadaValue *new_params = nada_cons(param_symbol_copy, params);
            nada_free(param_symbol_copy);  // Cons creates its own copy
            nada_free(params);             // Free the old list head
            params = new_params;           // Update params to point to the new list
            current_binding = nada_cdr(current_binding);
        }
        // params is now the reversed list of parameter symbols

        NadaValue *reversed_params = nada_reverse(params);  // Reverse to get correct order
        // Free the original reversed list (now pointed to by 'params')
        // before assigning the newly created correct-order list.
        nada_free(params);
        params = reversed_params;  // 'params' now holds the correct list

        // Create the loop function capturing the loop environment
        // Pass ownership of params and body copy to the function
        NadaValue *loop_func = nada_create_function(
            params,  // Pass params directly
            nada_deep_copy(body),
            loop_env  // Capture the current environment (adds ref)
        );
        // params list is now owned by loop_func, don't free here

        // Bind the loop function to its name *within* the loop environment
        // nada_env_set handles the self-reference ref count adjustment
        nada_env_set(loop_env, loop_name, loop_func);

        // Free the original loop_func value created above, *after* it's been copied.
        // Temporarily NULL the env pointer to prevent premature release of the
        // environment, which is now correctly referenced by the copy in the binding.
        if (loop_func->type == NADA_FUNC) {  // Safety check
            // We don't need to save/restore, just prevent release during this free
            loop_func->data.function.env = NULL;
        }
        nada_free(loop_func);  // Free the original loop_func value

        // Evaluate body
        NadaValue *result = nada_create_nil();
        NadaValue *current_expr = body;
        while (!nada_is_nil(current_expr)) {
            nada_free(result);  // Free previous result
            result = nada_eval(nada_car(current_expr), loop_env);
            if (result->type == NADA_ERROR) {  // Check for eval errors in body
                                               // Error occurred, result holds the error value.
                                               // Need to release env before returning.
                nada_env_release(loop_env);    // Release our scope reference
                nada_env_release(loop_env);    // Release the 'extra' scope
                return result;                 // Propagate error
            }
            current_expr = nada_cdr(current_expr);
        }

        // Release the environment references and return the last result
        nada_env_release(loop_env);  // Release our scope reference
        nada_env_release(loop_env);  // Release the 'extra' scope
        return result;
    } else {
        // Regular let
        NadaValue *bindings = first_arg;
        if (!nada_is_nil(bindings) && bindings->type != NADA_PAIR) {
            nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "let bindings must be a list");
            return nada_create_nil();
        }

        // Create a new environment
        NadaEnv *let_env = nada_env_create(env);

        // IMPORTANT: Define the body - this was missing!
        NadaValue *body = nada_cdr(args);

        // Evaluate binding expressions in the original env
        NadaValue *binding_list = bindings;
        while (!nada_is_nil(binding_list)) {
            NadaValue *binding = nada_car(binding_list);
            if (binding->type != NADA_PAIR ||
                nada_car(binding)->type != NADA_SYMBOL ||
                nada_is_nil(nada_cdr(binding)) ||
                !nada_is_nil(nada_cdr(nada_cdr(binding)))) {
                nada_report_error(NADA_ERROR_INVALID_ARGUMENT,
                                  "let binding must be (variable value)");
                nada_env_release(let_env);
                return nada_create_nil();
            }
            const char *var_name = nada_car(binding)->data.symbol;
            NadaValue *val_expr = nada_car(nada_cdr(binding));
            NadaValue *val = nada_eval(val_expr, env);

            nada_env_set(let_env, var_name, val);
            if (val->type == NADA_ERROR) {  // Check for eval errors
                nada_env_release(let_env);  // Release env before returning
                return val;                 // Propagate error
            }

            nada_free(val);
            binding_list = nada_cdr(binding_list);
        }

        // Evaluate body expressions in the new environment
        NadaValue *result = nada_create_nil();
        NadaValue *body_expr = body;

        while (!nada_is_nil(body_expr)) {
            // Free previous result before getting a new one
            nada_free(result);

            // Evaluate the next expression
            result = nada_eval(nada_car(body_expr), let_env);

            // Move to next expression
            body_expr = nada_cdr(body_expr);
        }

        // Make a copy of the result to return
        NadaValue *result_copy = nada_deep_copy(result);
        nada_free(result);

        // Before releasing let_env, find and fix circular references
        struct NadaBinding *binding = let_env->bindings;
        while (binding != NULL) {
            if (binding->value && binding->value->type == NADA_FUNC &&
                binding->value->data.function.env == let_env) {
                printf("Breaking circular reference in let-bound function: %s\n", binding->name);

                // Set function's env pointer to parent env and increment parent's ref count
                binding->value->data.function.env = let_env->parent;
                if (let_env->parent) {
                    nada_env_add_ref(let_env->parent);
                }
            }
            binding = binding->next;
        }

        // Force let_env to be fully cleaned up
        printf("Forcing cleanup of let_env #%d\n", let_env->id);
        let_env->ref_count = 1;  // Set to 1 so next release will free it
        nada_env_release(let_env);

        return result_copy;
    }
}

// Built-in special form: begin
NadaValue *builtin_begin(NadaValue *args, NadaEnv *env) {
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

// Built-in special form: set!
NadaValue *builtin_set(NadaValue *args, NadaEnv *env) {
    // Check that we have exactly two arguments: variable and value
    if (nada_is_nil(args) || nada_is_nil(nada_cdr(args)) ||
        !nada_is_nil(nada_cdr(nada_cdr(args)))) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "set! requires exactly 2 arguments");
        return nada_create_nil();
    }

    // Get the symbol to modify
    NadaValue *var = nada_car(args);
    if (var->type != NADA_SYMBOL) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "set! first argument must be a symbol");
        return nada_create_nil();
    }

    // Evaluate the value expression
    NadaValue *val = nada_eval(nada_car(nada_cdr(args)), env);

    // Find the binding in the environment (could be in parent environments)
    NadaEnv *current_env = env;
    int found = 0;

    while (current_env != NULL) {
        // Check if the variable exists in this environment
        struct NadaBinding *binding = current_env->bindings;
        while (binding != NULL) {
            if (strcmp(binding->name, var->data.symbol) == 0) {
                // Found the binding, update it
                nada_free(binding->value);
                binding->value = nada_deep_copy(val);
                found = 1;
                break;
            }
            binding = binding->next;
        }

        if (found) break;
        current_env = current_env->parent;
    }

    if (!found) {
        nada_report_error(NADA_ERROR_UNDEFINED_SYMBOL, "set! variable '%s' not found", var->data.symbol);
        nada_free(val);
        return nada_create_nil();
    }

    // Return the new value
    return val;
}
