#include <string.h>

#include "NadaEval.h"
#include "NadaError.h"
#include "NadaBuiltinCompare.h"

// Less than (<)
NadaValue *builtin_less_than(NadaValue *args, NadaEnv *env) {
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
NadaValue *builtin_less_equal(NadaValue *args, NadaEnv *env) {
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
NadaValue *builtin_greater_than(NadaValue *args, NadaEnv *env) {
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
NadaValue *builtin_greater_equal(NadaValue *args, NadaEnv *env) {
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
NadaValue *builtin_numeric_equal(NadaValue *args, NadaEnv *env) {
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
NadaValue *builtin_eq(NadaValue *args, NadaEnv *env) {
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
        case NADA_ERROR:
            // Errors with the same message are eq?
            result = (strcmp(first->data.error, second->data.error) == 0);
            break;
        }
    }

    nada_free(first);
    nada_free(second);

    return nada_create_bool(result);
}

// Helper function for recursive equality check
int values_equal(NadaValue *a, NadaValue *b) {
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
    case NADA_ERROR:
        return strcmp(a->data.error, b->data.error) == 0;
    default:
        return 0;
    }
}

// Recursive structural equality (equal?)
NadaValue *builtin_equal(NadaValue *args, NadaEnv *env) {
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
