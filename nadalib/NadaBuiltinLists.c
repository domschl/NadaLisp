#include <string.h>
#include <stdlib.h>

#include "NadaError.h"
#include "NadaValue.h"
#include "NadaEval.h"
#include "NadaBuiltinLists.h"

// Simple, straightforward car implementation
NadaValue *builtin_car(NadaValue *args, NadaEnv *env) {
    if (nada_is_nil(args) || !nada_is_nil(nada_cdr(args))) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "car requires exactly 1 argument");
        return nada_create_nil();
    }

    // Evaluate the argument
    NadaValue *arg = nada_eval(nada_car(args), env);

    // Check that it's a pair
    if (arg->type != NADA_PAIR) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "car called on non-pair");
        nada_free(arg);
        return nada_create_nil();
    }

    // Return a copy of the car value
    NadaValue *result = nada_deep_copy(arg->data.pair.car);
    nada_free(arg);
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

// Special map-car function that doesn't evaluate its argument
NadaValue *builtin_map_car(NadaValue *args, NadaEnv *env) {
    if (nada_is_nil(args) || !nada_is_nil(nada_cdr(args))) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "car requires exactly 1 argument");
        return nada_create_nil();
    }

    // Don't evaluate - just get the argument directly
    NadaValue *arg = nada_car(args);

    // Check that it's a pair
    if (arg->type != NADA_PAIR) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "car called on non-pair");
        return nada_create_nil();
    }

    // Return a copy of the car value
    return nada_deep_copy(arg->data.pair.car);
}

// Fixed map implementation that handles multiple lists correctly
NadaValue *builtin_map(NadaValue *args, NadaEnv *env) {
    // Check that we have at least two arguments
    if (nada_is_nil(args) || nada_is_nil(nada_cdr(args))) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "map requires a function and at least one list");
        return nada_create_nil();
    }

    // Get and evaluate the function argument
    NadaValue *func = nada_eval(nada_car(args), env);
    if (func->type != NADA_FUNC) {
        nada_report_error(NADA_ERROR_TYPE_ERROR, "map: first argument must be a function");
        nada_free(func);
        return nada_create_nil();
    }

    // Count the number of list arguments
    int list_count = 0;
    NadaValue *list_args = nada_cdr(args);
    NadaValue *current_list_arg = list_args;

    while (!nada_is_nil(current_list_arg)) {
        list_count++;
        current_list_arg = nada_cdr(current_list_arg);
    }

    // Evaluate all list arguments
    NadaValue **lists = malloc(list_count * sizeof(NadaValue *));
    if (!lists) {
        nada_report_error(NADA_ERROR_MEMORY, "Failed to allocate memory for map");
        nada_free(func);
        return nada_create_nil();
    }

    current_list_arg = list_args;
    for (int i = 0; i < list_count; i++) {
        lists[i] = nada_eval(nada_car(current_list_arg), env);
        current_list_arg = nada_cdr(current_list_arg);
    }

    // Check that all arguments are lists
    for (int i = 0; i < list_count; i++) {
        if (!nada_is_nil(lists[i]) && lists[i]->type != NADA_PAIR) {
            nada_report_error(NADA_ERROR_TYPE_ERROR, "map: all arguments after the function must be lists");

            // Clean up
            for (int j = 0; j < list_count; j++) {
                nada_free(lists[j]);
            }
            free(lists);
            nada_free(func);

            return nada_create_nil();
        }
    }

    // Create result list
    NadaValue *result = nada_create_nil();
    NadaValue *result_last = NULL;

    // Process elements until any list is exhausted
    int continue_mapping = 1;
    while (continue_mapping) {
        // Check if we need to stop (any list is empty)
        for (int i = 0; i < list_count; i++) {
            if (nada_is_nil(lists[i])) {
                continue_mapping = 0;
                break;
            }
        }

        if (!continue_mapping) break;

        // Build argument list for this iteration
        NadaValue *call_args = nada_create_nil();

        // Add one element from each list to the function arguments
        for (int i = list_count - 1; i >= 0; i--) {
            NadaValue *element = nada_car(lists[i]);
            NadaValue *new_call_args = nada_cons(element, call_args);
            nada_free(call_args);
            call_args = new_call_args;

            // Move to next element in this list
            NadaValue *next_list = nada_cdr(lists[i]);
            nada_free(lists[i]);
            lists[i] = next_list;
        }

        // Call the function with these arguments
        NadaValue *mapped_value;

        // Special handling for car
        if (func->data.function.builtin == builtin_car) {
            mapped_value = builtin_map_car(call_args, env);
        } else if (func->data.function.builtin != NULL) {
            mapped_value = func->data.function.builtin(call_args, env);
        } else {
            mapped_value = apply_function(func, call_args, env);
        }

        nada_free(call_args);

        // Add to result list
        if (nada_is_nil(result)) {
            result = nada_cons(mapped_value, nada_create_nil());
            result_last = result;
        } else {
            NadaValue *new_pair = nada_cons(mapped_value, nada_create_nil());
            result_last->data.pair.cdr = new_pair;
            result_last = new_pair;
        }

        nada_free(mapped_value);
    }

    // Clean up remaining lists
    free(lists);
    nada_free(func);

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
