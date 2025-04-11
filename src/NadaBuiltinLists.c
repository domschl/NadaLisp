#include <string.h>
#include <stdlib.h>

#include "NadaError.h"
#include "NadaEval.h"
#include "NadaBuiltinLists.h"

// Built-in function: car
NadaValue *builtin_car(NadaValue *args, NadaEnv *env) {
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
NadaValue *builtin_cdr(NadaValue *args, NadaEnv *env) {
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
NadaValue *builtin_cadr(NadaValue *args, NadaEnv *env) {
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
NadaValue *builtin_caddr(NadaValue *args, NadaEnv *env) {
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

// sublist: Extract a portion of a list
NadaValue *builtin_sublist(NadaValue *args, NadaEnv *env) {
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
NadaValue *builtin_list_ref(NadaValue *args, NadaEnv *env) {
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
NadaValue *builtin_map(NadaValue *args, NadaEnv *env) {
    if (nada_is_nil(args) || nada_is_nil(nada_cdr(args))) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT,
                          "map requires at least 2 arguments: function and list");
        return nada_create_nil();
    }

    // Get the function (first argument)
    NadaValue *func_arg = nada_eval(nada_car(args), env);

    // Check that it's a function
    if (func_arg->type != NADA_FUNC) {
        // If it's a symbol, try looking it up
        if (func_arg->type == NADA_SYMBOL) {
            NadaValue *func_val = nada_env_get(env, func_arg->data.symbol, 0);
            nada_free(func_arg);
            func_arg = func_val;

            // Check again if it's a function
            if (func_arg->type != NADA_FUNC) {
                nada_report_error(NADA_ERROR_TYPE_ERROR,
                                  "map requires a function as first argument");
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
        // Get the current element
        NadaValue *element = nada_car(current);
        NadaValue *mapped_value = NULL;

        if (func_arg->data.function.builtin) {
            // Check if the function is car, cdr, or similar list operations
            int is_list_op = 0;
            const char *func_name = NULL;

            func_name = get_builtin_name(func_arg->data.function.builtin);
            if (func_name != NULL) {
                if (strcmp(func_name, "car") == 0 ||
                    strcmp(func_name, "cdr") == 0 ||
                    strcmp(func_name, "cadr") == 0 ||
                    strcmp(func_name, "caddr") == 0 ||
                    strcmp(func_name, "list-ref") == 0) {
                    is_list_op = 1;
                }
            }

            if (is_list_op) {
                // For list operations, construct a quoted version of the element
                NadaValue *quote_sym = nada_create_symbol("quote");
                NadaValue *element_copy = nada_deep_copy(element);
                NadaValue *nil_value1 = nada_create_nil();
                NadaValue *quote_tail = nada_cons(element_copy, nil_value1);
                NadaValue *quoted_elem = nada_cons(quote_sym, quote_tail);
                NadaValue *nil_value2 = nada_create_nil();
                NadaValue *func_args = nada_cons(quoted_elem, nil_value2);

                // Apply the function
                mapped_value = func_arg->data.function.builtin(func_args, env);

                // Free all temporary values in reverse order of creation
                nada_free(func_args);
                nada_free(nil_value2);
                nada_free(quoted_elem);
                nada_free(quote_tail);
                nada_free(nil_value1);
                nada_free(element_copy);
                nada_free(quote_sym);
            } else {
                // For other built-in functions, evaluate the element first
                NadaValue *eval_element = nada_eval(element, env);
                NadaValue *nil_value = nada_create_nil();
                NadaValue *func_args = nada_cons(eval_element, nil_value);

                // Apply the function
                mapped_value = func_arg->data.function.builtin(func_args, env);

                // Free all temporary values
                nada_free(func_args);
                nada_free(nil_value);
                nada_free(eval_element);
            }
        } else {
            // For user-defined functions
            NadaValue *element_copy = nada_deep_copy(element);
            NadaValue *nil_value = nada_create_nil();
            NadaValue *func_call = nada_cons(element_copy, nil_value);

            // Apply the function
            mapped_value = apply_function(func_arg, func_call, env);

            // Free all temporary values
            nada_free(func_call);
            nada_free(nil_value);
            nada_free(element_copy);
        }

        // Handle error conditions
        if (mapped_value == NULL) {
            nada_free(mapped_items);
            nada_free(func_arg);
            nada_free(list_arg);
            return nada_create_nil();
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

// Cons function: Create a pair
NadaValue *builtin_cons(NadaValue *args, NadaEnv *env) {
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
NadaValue *builtin_list(NadaValue *args, NadaEnv *env) {
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

// Length function - count elements in a list
NadaValue *builtin_length(NadaValue *args, NadaEnv *env) {
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
