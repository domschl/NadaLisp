#include "NadaEval.h"
#include "NadaError.h"
#include "NadaBuiltinPredicates.h"

// Empty list test (null?)
NadaValue *builtin_null(NadaValue *args, NadaEnv *env) {
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
NadaValue *builtin_integer_p(NadaValue *args, NadaEnv *env) {
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
NadaValue *builtin_number_p(NadaValue *args, NadaEnv *env) {
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
NadaValue *builtin_string_p(NadaValue *args, NadaEnv *env) {
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
NadaValue *builtin_symbol_p(NadaValue *args, NadaEnv *env) {
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
NadaValue *builtin_boolean_p(NadaValue *args, NadaEnv *env) {
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
NadaValue *builtin_pair_p(NadaValue *args, NadaEnv *env) {
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
NadaValue *builtin_function_p(NadaValue *args, NadaEnv *env) {
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
int is_proper_list(NadaValue *v) {
    if (v->type == NADA_NIL) return 1;
    if (v->type != NADA_PAIR) return 0;
    return is_proper_list(v->data.pair.cdr);
}

// List predicate (list?)
NadaValue *builtin_list_p(NadaValue *args, NadaEnv *env) {
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
NadaValue *builtin_atom_p(NadaValue *args, NadaEnv *env) {
    if (nada_is_nil(args) || !nada_is_nil(nada_cdr(args))) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "atom? requires exactly 1 argument");
        return nada_create_bool(0);
    }

    NadaValue *val = nada_eval(nada_car(args), env);
    int result = (val->type != NADA_PAIR && val->type != NADA_NIL);
    nada_free(val);
    return nada_create_bool(result);
}

// Error predicate (error?)
NadaValue *builtin_error_p(NadaValue *args, NadaEnv *env) {
    if (nada_is_nil(args) || !nada_is_nil(nada_cdr(args))) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "error? requires exactly 1 argument");
        return nada_create_bool(0);
    }

    NadaValue *val = nada_eval(nada_car(args), env);
    int result = (val->type == NADA_ERROR);
    nada_free(val);
    return nada_create_bool(result);
}
