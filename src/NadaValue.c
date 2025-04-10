#include "NadaValue.h"
#include "NadaEval.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Initialize counters
static int value_allocations = 0;
static int value_frees = 0;
static int current_values = 0;

// Functions to increment counters
void nada_increment_allocations(void) {
    value_allocations++;
    current_values++;
}

void nada_increment_frees(void) {
    value_frees++;
    current_values--;
}

// Reset for accurate tracking
void nada_memory_reset() {
    value_allocations = 0;
    value_frees = 0;
    current_values = 0;
}

// Create a new number value
NadaValue *nada_create_num(NadaNum *num) {
    NadaValue *val = malloc(sizeof(NadaValue));
    if (val == NULL) {
        fprintf(stderr, "Error: Out of memory\n");
        exit(1);
    }
    val->type = NADA_NUM;
    val->data.number = nada_num_copy(num);  // Make a copy to own the number
    nada_increment_allocations();
    return val;
}

NadaValue *nada_create_num_from_int(int value) {
    NadaNum *num = nada_num_from_int(value);
    NadaValue *val = nada_create_num(num);
    nada_num_free(num);  // Free the original since nada_create_num makes a copy
    return val;
}

NadaValue *nada_create_num_from_string(const char *str) {
    NadaNum *num = nada_num_from_string(str);
    if (num == NULL) {
        // Invalid number string
        return nada_create_nil();
    }
    NadaValue *val = nada_create_num(num);
    nada_num_free(num);  // Free the original
    return val;
}

// Create a new string value
NadaValue *nada_create_string(const char *str) {
    NadaValue *val = malloc(sizeof(NadaValue));
    val->type = NADA_STRING;
    val->data.string = strdup(str);
    nada_increment_allocations();
    return val;
}

// Create a new symbol
NadaValue *nada_create_symbol(const char *name) {
    NadaValue *val = malloc(sizeof(NadaValue));
    val->type = NADA_SYMBOL;
    val->data.symbol = strdup(name);
    nada_increment_allocations();
    return val;
}

// Create nil value
NadaValue *nada_create_nil(void) {
    NadaValue *val = malloc(sizeof(NadaValue));
    val->type = NADA_NIL;
    nada_increment_allocations();
    return val;
}

// Create a boolean value
NadaValue *nada_create_bool(int boolean) {
    NadaValue *val = malloc(sizeof(NadaValue));
    val->type = NADA_BOOL;
    val->data.boolean = boolean ? 1 : 0;
    nada_increment_allocations();
    return val;
}

// Create a cons cell / pair
NadaValue *nada_cons(NadaValue *car, NadaValue *cdr) {
    NadaValue *pair = malloc(sizeof(NadaValue));
    pair->type = NADA_PAIR;
    // Make deep copies of car and cdr
    pair->data.pair.car = nada_deep_copy(car);
    pair->data.pair.cdr = nada_deep_copy(cdr);
    nada_increment_allocations();
    return pair;
}

// Create a function value
NadaValue *nada_create_function(NadaValue *params, NadaValue *body, NadaEnv *env) {
    NadaValue *val = malloc(sizeof(NadaValue));
    if (!val) return NULL;

    val->type = NADA_FUNC;
    val->data.function.params = params;
    val->data.function.body = body;
    val->data.function.env = env;
    val->data.function.builtin = NULL;
    
    // Add a reference to the environment
    nada_env_add_ref(env);
    
    nada_increment_allocations();
    return val;
}

// Get the car (first element) of a pair
NadaValue *nada_car(NadaValue *pair) {
    if (pair->type != NADA_PAIR) {
        fprintf(stderr, "Error: car called on non-pair\n");
        return nada_create_nil();
    }
    return pair->data.pair.car;
}

// Get the cdr (rest of the list) of a pair
NadaValue *nada_cdr(NadaValue *pair) {
    if (pair->type != NADA_PAIR) {
        fprintf(stderr, "Error: cdr called on non-pair\n");
        return nada_create_nil();
    }
    return pair->data.pair.cdr;
}

// Check if a value is nil
int nada_is_nil(NadaValue *val) {
    return val->type == NADA_NIL;
}

// Free a value and its children
void nada_free(NadaValue *val) {
    if (val == NULL) return;

    switch (val->type) {
    case NADA_NUM:
        nada_num_free(val->data.number);
        break;
    case NADA_STRING:
        free(val->data.string);
        break;
    case NADA_SYMBOL:
        free(val->data.symbol);
        break;
    case NADA_PAIR:
        if (val->data.pair.car) {
            nada_free(val->data.pair.car);
            val->data.pair.car = NULL; // Prevent double-free
        }
        if (val->data.pair.cdr) {
            nada_free(val->data.pair.cdr);
            val->data.pair.cdr = NULL; // Prevent double-free
        }
        break;
    case NADA_FUNC:
        if (val->data.function.params) {
            nada_free(val->data.function.params);
            val->data.function.params = NULL; // Prevent double-free
        }
        if (val->data.function.body) {
            nada_free(val->data.function.body);
            val->data.function.body = NULL; // Prevent double-free
        }
        
        // Release the environment (will only free if ref_count drops to 0)
        if (val->data.function.env) {
            nada_env_release(val->data.function.env);
            val->data.function.env = NULL; // Prevent double-free
        }
        break;
    case NADA_NIL:
        // No special cleanup needed for nil
        break;
    case NADA_BOOL:
        // No special cleanup needed for boolean
        break;
    }

    free(val);
    nada_increment_frees();
}

// Print a value (for debugging and REPL output)
void nada_print(NadaValue *val) {
    if (val == NULL) {
        printf("NULL");
        return;
    }

    switch (val->type) {
    case NADA_NUM: {
        char *str = nada_num_to_string(val->data.number);
        printf("%s", str);
        free(str);
        break;
    }
    case NADA_STRING:
        printf("\"%s\"", val->data.string);
        break;
    case NADA_SYMBOL:
        printf("%s", val->data.symbol);
        break;
    case NADA_NIL:
        printf("()");
        break;
    case NADA_PAIR:
        printf("(");
        nada_print(val->data.pair.car);

        // Print rest of the list
        NadaValue *rest = val->data.pair.cdr;
        while (rest->type == NADA_PAIR) {
            printf(" ");
            nada_print(rest->data.pair.car);
            rest = rest->data.pair.cdr;
        }

        // Handle improper lists
        if (rest->type != NADA_NIL) {
            printf(" . ");
            nada_print(rest);
        }

        printf(")");
        break;
    case NADA_FUNC:
        printf("#<function>");
        break;
    case NADA_BOOL:
        printf("%s", val->data.boolean ? "#t" : "#f");
        break;
    }
}

// Deep copy a value and all its children
NadaValue *nada_deep_copy(NadaValue *val) {
    if (val == NULL) return NULL;

    NadaValue *result = malloc(sizeof(NadaValue));
    result->type = val->type;

    switch (val->type) {
    case NADA_NUM:
        result->data.number = nada_num_copy(val->data.number);
        break;
    case NADA_STRING:
        result->data.string = strdup(val->data.string);
        break;
    case NADA_SYMBOL:
        result->data.symbol = strdup(val->data.symbol);
        break;
    case NADA_NIL:
        // No additional initialization needed for nil
        break;
    case NADA_PAIR:
        result->data.pair.car = nada_deep_copy(val->data.pair.car);
        result->data.pair.cdr = nada_deep_copy(val->data.pair.cdr);
        break;
    case NADA_FUNC:
        result->data.function.params = nada_deep_copy(val->data.function.params);
        result->data.function.body = nada_deep_copy(val->data.function.body);
        result->data.function.env = val->data.function.env;          // Share environment
        result->data.function.builtin = val->data.function.builtin;  // Copy the built-in function pointer
        
        // Add reference to shared environment
        if (result->data.function.env) {
            nada_env_add_ref(result->data.function.env);
        }
        break;
    case NADA_BOOL:
        result->data.boolean = val->data.boolean;
        break;
    }

    nada_increment_allocations();
    return result;
}

/*
// Reverse a list in-place
NadaValue *nada_reverse(NadaValue *list) {
    NadaValue *result = nada_create_nil();

    while (!nada_is_nil(list)) {
        NadaValue *head = nada_car(list);
        NadaValue *tail = nada_cdr(list);

        result = nada_cons(head, result);
        list = tail;
    }

    return result;
}
*/

// Fix for nada_reverse function
NadaValue *nada_reverse(NadaValue *list) {
    // Create a temporary variable for the nil value instead of passing directly
    NadaValue *result = nada_create_nil();
    NadaValue *current = list;

    while (current->type == NADA_PAIR) {
        NadaValue *new_result = nada_cons(nada_car(current), result);
        nada_free(result);  // Free the old result
        result = new_result;
        current = nada_cdr(current);
    }

    return result;
}

// Updated memory report
void nada_memory_report() {
    printf("Memory report: %d allocations, %d frees, %d active, %d leak(s)\n",
           value_allocations, value_frees, current_values,
           value_allocations - value_frees);
}