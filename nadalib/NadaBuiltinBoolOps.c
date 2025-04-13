#include "NadaEval.h"
#include "NadaError.h"
#include "NadaBuiltinBoolOps.h"

// Logical negation (not)
NadaValue *builtin_not(NadaValue *args, NadaEnv *env) {
    if (nada_is_nil(args) || !nada_is_nil(nada_cdr(args))) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "not requires exactly 1 argument");
        return nada_create_bool(0);
    }

    NadaValue *arg = nada_eval(nada_car(args), env);

    // In standard Scheme, only #f is falsy, everything else is truthy
    // (including empty lists/nil)
    int is_falsy = (arg->type == NADA_BOOL && arg->data.boolean == 0);

    nada_free(arg);

    // Return the logical negation
    return nada_create_bool(is_falsy);
}

// Built-in special form: or
NadaValue *builtin_or(NadaValue *args, NadaEnv *env) {
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
            NadaValue *result_copy = nada_deep_copy(result);
            nada_free(result);
            return result_copy;
        }

        // Otherwise, move to next argument
        expr = nada_cdr(expr);
    }

    // All arguments were falsy, return the last result (which is falsy)
    NadaValue *result_copy = nada_deep_copy(result);
    nada_free(result);
    return result_copy;
}

// Built-in special form: and
NadaValue *builtin_and(NadaValue *args, NadaEnv *env) {
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
            NadaValue *result_copy = nada_deep_copy(result);
            nada_free(result);
            return result_copy;
        }

        // Otherwise, move to next argument
        expr = nada_cdr(expr);
    }

    // All arguments were truthy, return the last result (which is truthy)
    NadaValue *result_copy = nada_deep_copy(result);
    nada_free(result);
    return result_copy;
}
