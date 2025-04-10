#include <stdlib.h>
#include "NadaBuiltinMath.h"

// Addition (+)
NadaValue *builtin_add(NadaValue *args, NadaEnv *env) {
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
NadaValue *builtin_subtract(NadaValue *args, NadaEnv *env) {
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
NadaValue *builtin_multiply(NadaValue *args, NadaEnv *env) {
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
NadaValue *builtin_divide(NadaValue *args, NadaEnv *env) {
    if (nada_is_nil(args)) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "'/' requires at least one argument");
        return nada_create_num_from_int(0);
    }

    // Start with first argument
    NadaValue *first = nada_eval(nada_car(args), env);
    if (first->type != NADA_NUM) {
        nada_report_error(NADA_ERROR_TYPE_ERROR, "'/' requires number arguments");
        nada_free(first);
        return nada_create_num_from_int(0);
    }

    NadaNum *result = nada_num_copy(first->data.number);
    nada_free(first);

    NadaValue *rest = nada_cdr(args);
    if (nada_is_nil(rest)) {
        // Unary division (1/x)
        if (nada_num_is_zero(result)) {
            nada_report_error(NADA_ERROR_DIVISION_BY_ZERO, "division by zero");
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
            nada_report_error(NADA_ERROR_TYPE_ERROR, "'/' requires number arguments");
            nada_num_free(result);
            nada_free(arg);
            return nada_create_num_from_int(0);
        }

        if (nada_num_is_zero(arg->data.number)) {
            nada_report_error(NADA_ERROR_DIVISION_BY_ZERO, "division by zero");
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

// Built-in function: modulo
NadaValue *builtin_modulo(NadaValue *args, NadaEnv *env) {
    if (nada_is_nil(args) || nada_is_nil(nada_cdr(args)) ||
        !nada_is_nil(nada_cdr(nada_cdr(args)))) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "modulo requires exactly 2 arguments");
        return nada_create_nil();
    }

    NadaValue *a = nada_eval(nada_car(args), env);
    NadaValue *b = nada_eval(nada_car(nada_cdr(args)), env);

    if (a->type != NADA_NUM || b->type != NADA_NUM) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "modulo arguments must be numbers");
        nada_free(a);
        nada_free(b);
        return nada_create_nil();
    }

    // Check for division by zero
    if (nada_num_is_zero(b->data.number)) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "division by zero");
        nada_free(a);
        nada_free(b);
        return nada_create_nil();
    }

    // Perform modulo operation
    NadaNum *result_num = nada_num_modulo(a->data.number, b->data.number);

    // Create result
    NadaValue *result = nada_create_num(result_num);

    // Free temporary value - this is crucial
    nada_num_free(result_num);

    // Free arguments
    nada_free(a);
    nada_free(b);

    return result;
}
