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

// Create non-evaluating versions of the list accessor functions for map
NadaValue *map_car(NadaValue *args, NadaEnv *env) {
    if (nada_is_nil(args) || !nada_is_nil(nada_cdr(args))) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "car requires exactly 1 argument");
        return nada_create_nil();
    }

    // Don't evaluate - just get the argument directly
    NadaValue *arg = nada_car(args);

    if (arg->type != NADA_PAIR) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "car called on non-pair");
        return nada_create_nil();
    }

    return nada_deep_copy(arg->data.pair.car);
}

NadaValue *map_cdr(NadaValue *args, NadaEnv *env) {
    if (nada_is_nil(args) || !nada_is_nil(nada_cdr(args))) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "cdr requires exactly 1 argument");
        return nada_create_nil();
    }

    NadaValue *arg = nada_car(args);

    if (arg->type != NADA_PAIR) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "cdr called on non-pair");
        return nada_create_nil();
    }

    return nada_deep_copy(arg->data.pair.cdr);
}

NadaValue *map_cadr(NadaValue *args, NadaEnv *env) {
    // Similar implementation for cadr without eval
    if (nada_is_nil(args) || !nada_is_nil(nada_cdr(args))) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "cadr requires exactly 1 argument");
        return nada_create_nil();
    }

    NadaValue *arg = nada_car(args);

    if (arg->type != NADA_PAIR) {
        nada_report_error(NADA_ERROR_TYPE_ERROR, "cadr requires a list argument");
        return nada_create_nil();
    }

    NadaValue *cdr_val = nada_cdr(arg);

    if (cdr_val->type != NADA_PAIR) {
        nada_report_error(NADA_ERROR_TYPE_ERROR, "list has no second element");
        return nada_create_nil();
    }

    return nada_deep_copy(nada_car(cdr_val));
}

NadaValue *map_caddr(NadaValue *args, NadaEnv *env) {
    // Similar implementation for caddr without eval
    // Implementation similar to map_cadr but getting third element
    if (nada_is_nil(args) || !nada_is_nil(nada_cdr(args))) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "caddr requires exactly 1 argument");
        return nada_create_nil();
    }

    NadaValue *arg = nada_car(args);

    if (arg->type != NADA_PAIR) {
        nada_report_error(NADA_ERROR_TYPE_ERROR, "caddr requires a list argument");
        return nada_create_nil();
    }

    NadaValue *cdr_val = nada_cdr(arg);
    if (cdr_val->type != NADA_PAIR) {
        nada_report_error(NADA_ERROR_TYPE_ERROR, "list has no second element");
        return nada_create_nil();
    }

    NadaValue *cddr_val = nada_cdr(cdr_val);
    if (cddr_val->type != NADA_PAIR) {
        nada_report_error(NADA_ERROR_TYPE_ERROR, "list has no third element");
        return nada_create_nil();
    }

    return nada_deep_copy(nada_car(cddr_val));
}

// Fixed map implementation that handles multiple lists correctly
NadaValue *builtin_map(NadaValue *args, NadaEnv *env) {
    // Check that we have at least 2 arguments
    if (nada_is_nil(args) || nada_is_nil(nada_cdr(args))) {
        nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "map requires at least 2 arguments");
        return nada_create_nil();
    }

    // Evaluate the first argument to get the function
    NadaValue *func = nada_eval(nada_car(args), env);
    if (func->type != NADA_FUNC) {
        nada_report_error(NADA_ERROR_TYPE_ERROR, "map requires a function as first argument");
        nada_free(func);
        return nada_create_nil();
    }

    // Get all list arguments (there may be multiple)
    int list_count = 0;
    NadaValue **list_args = NULL;
    NadaValue *current_arg_ptr = nada_cdr(args);

    // First, count how many list arguments we have
    while (!nada_is_nil(current_arg_ptr)) {
        list_count++;
        current_arg_ptr = nada_cdr(current_arg_ptr);
    }

    // Allocate array for evaluated list arguments
    list_args = malloc(list_count * sizeof(NadaValue *));
    if (!list_args) {
        nada_report_error(NADA_ERROR_OUT_OF_MEMORY, "Out of memory in map");
        nada_free(func);
        return nada_create_nil();
    }

    // Evaluate each list argument
    current_arg_ptr = nada_cdr(args);
    for (int i = 0; i < list_count; i++) {
        list_args[i] = nada_eval(nada_car(current_arg_ptr), env);

        // Check that it's actually a list
        if (!nada_is_nil(list_args[i]) && list_args[i]->type != NADA_PAIR) {
            nada_report_error(NADA_ERROR_TYPE_ERROR, "map requires list arguments");
            // Clean up
            nada_free(func);
            for (int j = 0; j <= i; j++) {
                nada_free(list_args[j]);
            }
            free(list_args);
            return nada_create_nil();
        }

        current_arg_ptr = nada_cdr(current_arg_ptr);
    }

    // Count elements in the first list to determine how many iterations
    int count = 0;
    NadaValue *current = list_args[0];
    while (current->type == NADA_PAIR) {
        count++;
        current = nada_cdr(current);
    }

    // Initialize the result to an empty list
    NadaValue *result = nada_create_nil();

    // Special handling for list functions (car, cdr, etc.)
    if ((func->data.function.builtin == builtin_car ||
         func->data.function.builtin == builtin_cdr ||
         func->data.function.builtin == builtin_cadr ||
         func->data.function.builtin == builtin_caddr) &&
        list_count == 1) {

        // Process each element of the list
        current = list_args[0];
        for (int i = 0; i < count; i++) {
            NadaValue *element = nada_car(current);
            NadaValue *mapped_result = NULL;

            if (func->data.function.builtin == builtin_car) {
                if (element->type == NADA_PAIR) {
                    mapped_result = nada_deep_copy(element->data.pair.car);
                } else {
                    nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "car called on non-pair");
                    mapped_result = nada_create_nil();
                }
            } else if (func->data.function.builtin == builtin_cdr) {
                if (element->type == NADA_PAIR) {
                    mapped_result = nada_deep_copy(element->data.pair.cdr);
                } else {
                    nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "cdr called on non-pair");
                    mapped_result = nada_create_nil();
                }
            } else if (func->data.function.builtin == builtin_cadr) {
                if (element->type == NADA_PAIR) {
                    NadaValue *cdr_val = nada_cdr(element);
                    if (cdr_val->type == NADA_PAIR) {
                        mapped_result = nada_deep_copy(nada_car(cdr_val));
                    } else {
                        mapped_result = nada_create_nil();
                    }
                } else {
                    mapped_result = nada_create_nil();
                }
            } else if (func->data.function.builtin == builtin_caddr) {
                if (element->type == NADA_PAIR) {
                    NadaValue *cdr_val = nada_cdr(element);
                    if (cdr_val->type == NADA_PAIR) {
                        NadaValue *cddr_val = nada_cdr(cdr_val);
                        if (cddr_val->type == NADA_PAIR) {
                            mapped_result = nada_deep_copy(nada_car(cddr_val));
                        } else {
                            mapped_result = nada_create_nil();
                        }
                    } else {
                        mapped_result = nada_create_nil();
                    }
                } else {
                    mapped_result = nada_create_nil();
                }
            }

            // Add the result to our list (in reverse)
            NadaValue *new_result = nada_cons(mapped_result, result);
            nada_free(mapped_result);
            nada_free(result);
            result = new_result;

            // Move to next element
            current = nada_cdr(current);
        }
    } else {
        // Regular case - for each position in the lists
        for (int i = 0; i < count; i++) {
            // Create a custom environment for this function call
            NadaEnv *call_env = nada_env_create(env);

            // For each list, get the ith element
            NadaValue *func_args = nada_create_nil();
            int all_lists_valid = 1;

            for (int j = list_count - 1; j >= 0; j--) {
                // Get the ith element of list j
                NadaValue *list_j = list_args[j];
                for (int k = 0; k < i && list_j->type == NADA_PAIR; k++) {
                    list_j = nada_cdr(list_j);
                }

                if (list_j->type == NADA_PAIR) {
                    // Extract the element at this position and add it to our args
                    NadaValue *element = nada_deep_copy(nada_car(list_j));
                    NadaValue *new_args = nada_cons(element, func_args);
                    nada_free(element);
                    nada_free(func_args);
                    func_args = new_args;
                } else {
                    all_lists_valid = 0;
                    break;
                }
            }

            if (!all_lists_valid) {
                nada_free(func_args);
                nada_env_release(call_env);
                break;
            }

            // If we have a lambda function, bind parameters directly
            NadaValue *mapped_result = NULL;

            if (func->data.function.builtin == NULL) {
                // For user-defined lambda, bind parameters directly
                NadaValue *params = func->data.function.params;
                NadaValue *body = func->data.function.body;
                NadaValue *current_param = params;
                NadaValue *current_arg = func_args;

                // Bind each parameter without re-evaluating
                while (!nada_is_nil(current_param) && !nada_is_nil(current_arg)) {
                    if (current_param->type != NADA_PAIR ||
                        current_param->data.pair.car->type != NADA_SYMBOL) {
                        nada_report_error(NADA_ERROR_INVALID_ARGUMENT, "invalid parameter list");
                        mapped_result = nada_create_nil();
                        break;
                    }

                    // Bind parameter directly to argument without evaluation
                    nada_env_set(call_env,
                                 current_param->data.pair.car->data.symbol,
                                 nada_car(current_arg));

                    // Move to next param and arg
                    current_param = nada_cdr(current_param);
                    current_arg = nada_cdr(current_arg);
                }

                // Evaluate body expressions
                mapped_result = nada_create_nil();
                NadaValue *current_expr = body;

                while (!nada_is_nil(current_expr)) {
                    nada_free(mapped_result);
                    mapped_result = nada_eval(nada_car(current_expr), call_env);
                    current_expr = nada_cdr(current_expr);
                }
            } else {
                // For built-in functions, use the builtin function directly
                mapped_result = func->data.function.builtin(func_args, env);
            }

            // Add the result to our list (in reverse)
            NadaValue *new_result = nada_cons(mapped_result, result);
            nada_free(mapped_result);
            nada_free(result);
            result = new_result;

            // Clean up
            nada_free(func_args);
            nada_env_release(call_env);
        }
    }

    // Reverse the result to get correct order
    NadaValue *final_result = nada_reverse(result);
    nada_free(result);

    // Clean up
    nada_free(func);
    for (int i = 0; i < list_count; i++) {
        nada_free(list_args[i]);
    }
    free(list_args);

    return final_result;
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
