#include <stdlib.h>
#include <string.h>

#include "NadaEval.h"
// #include "NadaNum.h"
#include "NadaError.h"
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

// Built-in function: remainder
NadaValue *builtin_remainder(NadaValue *args, NadaEnv *env) {
    if (nada_is_nil(args) || nada_is_nil(nada_cdr(args)) ||
        !nada_is_nil(nada_cdr(nada_cdr(args)))) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "remainder requires exactly 2 arguments");
        return nada_create_nil();
    }

    NadaValue *a = nada_eval(nada_car(args), env);
    NadaValue *b = nada_eval(nada_car(nada_cdr(args)), env);

    if (a->type != NADA_NUM || b->type != NADA_NUM) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "remainder arguments must be numbers");
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

    // Perform remainder operation
    NadaNum *result_num = nada_num_remainder(a->data.number, b->data.number);

    // Create result
    NadaValue *result = nada_create_num(result_num);

    // Free temporary value - this is crucial
    nada_num_free(result_num);

    // Free arguments
    nada_free(a);
    nada_free(b);

    return result;
}

// Built-in function: expt (exponentiation)
NadaValue *builtin_expt(NadaValue *args, NadaEnv *env) {
    if (nada_is_nil(args) || nada_is_nil(nada_cdr(args)) ||
        !nada_is_nil(nada_cdr(nada_cdr(args)))) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "expt requires exactly 2 arguments");
        return nada_create_nil();
    }

    NadaValue *base = nada_eval(nada_car(args), env);
    NadaValue *exponent = nada_eval(nada_car(nada_cdr(args)), env);

    if (base->type != NADA_NUM || exponent->type != NADA_NUM) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "expt arguments must be numbers");
        nada_free(base);
        nada_free(exponent);
        return nada_create_nil();
    }

    // Use encapsulated functions instead of direct structure access
    if (!nada_num_is_integer(exponent->data.number)) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT,
                          "expt: non-integer exponents require symbolic handling");
        nada_free(base);
        nada_free(exponent);
        return nada_create_nil();
    }

    // Perform exponentiation - letting NadaNum handle all internal checks
    NadaNum *result_num = NULL;

    // Convert to integer using nada_num API
    int exp_int = nada_num_to_int(exponent->data.number);

    // Call exponentiation function which should handle all error cases internally
    result_num = nada_num_int_expt(base->data.number, exp_int);

    if (result_num == NULL) {
        // Error occurred in the computation (e.g., 0^negative)
        // Error already reported by nada_num_int_expt
        nada_free(base);
        nada_free(exponent);
        return nada_create_nil();
    }

    // Create result
    NadaValue *result = nada_create_num(result_num);

    // Free temporary value
    nada_num_free(result_num);

    // Free arguments
    nada_free(base);
    nada_free(exponent);

    return result;
}

// Return the numerator of a rational number
NadaValue *builtin_numerator(NadaValue *args, NadaEnv *env) {
    // Check argument count
    if (nada_is_nil(args) || !nada_is_nil(nada_cdr(args))) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "numerator requires exactly one argument");
        return nada_create_nil();
    }

    // Evaluate the argument
    NadaValue *arg = nada_eval(nada_car(args), env);
    if (arg->type != NADA_NUM) {
        nada_report_error(NADA_ERROR_TYPE_ERROR, "numerator requires a number argument");
        nada_free(arg);
        return nada_create_nil();
    }

    // Get the numerator as a string
    char *num_str = nada_num_get_numerator(arg->data.number);
    if (!num_str) {
        nada_free(arg);
        return nada_create_nil();
    }

    // Create a new number with the numerator and denominator 1
    NadaNum *num = nada_num_from_fraction(num_str, "1");
    free(num_str);

    // Get the sign from the original number
    int sign = nada_num_get_sign(arg->data.number);
    nada_free(arg);

    // Create the result value
    NadaValue *result = nada_create_num(num);

    // Apply the sign (positive numerator is already handled)
    if (sign < 0 && !nada_num_is_zero(num)) {
        // Create a negative number
        NadaNum *neg_num = nada_num_negate(num);
        nada_num_free(num);

        // Update the result
        nada_free(result);
        result = nada_create_num(neg_num);
        nada_num_free(neg_num);
    } else {
        nada_num_free(num);
    }

    return result;
}

// Return the denominator of a rational number
NadaValue *builtin_denominator(NadaValue *args, NadaEnv *env) {
    // Check argument count
    if (nada_is_nil(args) || !nada_is_nil(nada_cdr(args))) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "denominator requires exactly one argument");
        return nada_create_nil();
    }

    // Evaluate the argument
    NadaValue *arg = nada_eval(nada_car(args), env);
    if (arg->type != NADA_NUM) {
        nada_report_error(NADA_ERROR_TYPE_ERROR, "denominator requires a number argument");
        nada_free(arg);
        return nada_create_nil();
    }

    // Get the denominator as a string
    char *denom_str = nada_num_get_denominator(arg->data.number);
    if (!denom_str) {
        nada_free(arg);
        return nada_create_nil();
    }

    // Create a new number with the denominator
    NadaNum *num = nada_num_from_int(atoi(denom_str));
    free(denom_str);
    nada_free(arg);

    // Create the result value
    NadaValue *result = nada_create_num(num);
    nada_num_free(num);

    return result;
}

// Return the sign of a number (1 for positive, -1 for negative)
NadaValue *builtin_sign(NadaValue *args, NadaEnv *env) {
    // Check argument count
    if (nada_is_nil(args) || !nada_is_nil(nada_cdr(args))) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "sign requires exactly one argument");
        return nada_create_nil();
    }

    // Evaluate the argument
    NadaValue *arg = nada_eval(nada_car(args), env);
    if (arg->type != NADA_NUM) {
        nada_report_error(NADA_ERROR_TYPE_ERROR, "sign requires a number argument");
        nada_free(arg);
        return nada_create_nil();
    }

    // Get the sign
    int sign = nada_num_get_sign(arg->data.number);
    nada_free(arg);

    // Create the result value (integer)
    NadaValue *result = nada_create_num_from_int(sign);
    return result;
}

// Return a list of prime factors of the numerator (if the number is an integer)
NadaValue *builtin_factor(NadaValue *args, NadaEnv *env) {
    // Check argument count
    if (nada_is_nil(args) || !nada_is_nil(nada_cdr(args))) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "factor requires exactly one argument");
        return nada_create_nil();
    }

    // Evaluate the argument
    NadaValue *arg = nada_eval(nada_car(args), env);
    if (arg->type != NADA_NUM) {
        nada_report_error(NADA_ERROR_TYPE_ERROR, "factor requires a number argument");
        nada_free(arg);
        return nada_create_nil();
    }

    // Check if the number is an integer
    if (!nada_num_is_integer(arg->data.number)) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "factor requires an integer argument");
        nada_free(arg);
        return nada_create_nil();
    }

    // Get the numerator as string (need to free this)
    char *num_str = nada_num_get_numerator(arg->data.number);
    if (!num_str) {
        nada_free(arg);
        return nada_create_nil();
    }

    // If the number is 0 or 1, return an empty list
    if (nada_num_is_zero(arg->data.number) ||
        (strcmp(num_str, "1") == 0 && nada_num_get_sign(arg->data.number) > 0)) {
        free(num_str);  // Free the numerator string
        nada_free(arg);
        return nada_create_nil();  // Empty list for 0 and 1
    }

    // Handle -1 specifically
    if (strcmp(num_str, "1") == 0 && nada_num_get_sign(arg->data.number) < 0) {
        free(num_str);  // Free the numerator string
        nada_free(arg);
        // Return a list containing just -1
        NadaValue *neg_one = nada_create_num_from_int(-1);
        // Create nil separately so we can free it
        NadaValue *nil = nada_create_nil();
        NadaValue *cons = nada_cons(neg_one, nil);
        nada_free(neg_one);
        nada_free(nil);  // Free the nil we created
        return cons;
    }

    // Remember if the number is negative
    int is_negative = nada_num_get_sign(arg->data.number) < 0;

    // We're done with the numerator string
    free(num_str);

    // Factor the number
    size_t factor_count = 0;
    NadaNum **factors = nada_num_factor_numerator(arg->data.number, &factor_count);

    // Create the result list
    NadaValue *result = nada_create_nil();

    // If factors is NULL, return the empty list
    if (!factors) {
        nada_free(arg);
        return result;
    }

    // Build the list from the array of factors (in reverse to get correct order)
    for (int i = (int)factor_count - 1; i >= 0; i--) {
        NadaValue *factor = nada_create_num(factors[i]);
        NadaValue *new_result = nada_cons(factor, result);
        nada_free(factor);
        nada_free(result);
        result = new_result;

        // Free the factor
        nada_num_free(factors[i]);
    }

    // Free the array itself
    free(factors);

    // Now, if the number was negative, prepend -1 to the list
    if (is_negative) {
        // Add -1 as the first factor (BEFORE other factors)
        NadaValue *neg_one = nada_create_num_from_int(-1);
        NadaValue *new_result = nada_cons(neg_one, result);
        nada_free(neg_one);
        nada_free(result);
        result = new_result;
    }

    nada_free(arg);
    return result;
}
