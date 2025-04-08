#include "NadaValue.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Create a new integer value
NadaValue *nada_create_int(int value) {
    NadaValue *val = malloc(sizeof(NadaValue));
    val->type = NADA_INT;
    val->data.integer = value;
    return val;
}

// Create a new string value
NadaValue *nada_create_string(const char *str) {
    NadaValue *val = malloc(sizeof(NadaValue));
    val->type = NADA_STRING;
    val->data.string = strdup(str);
    return val;
}

// Create a new symbol
NadaValue *nada_create_symbol(const char *name) {
    NadaValue *val = malloc(sizeof(NadaValue));
    val->type = NADA_SYMBOL;
    val->data.symbol = strdup(name);
    return val;
}

// Create nil value
NadaValue *nada_create_nil(void) {
    NadaValue *val = malloc(sizeof(NadaValue));
    val->type = NADA_NIL;
    return val;
}

// Create a boolean value
NadaValue *nada_create_bool(int boolean) {
    NadaValue *val = malloc(sizeof(NadaValue));
    val->type = NADA_BOOL;
    val->data.boolean = boolean ? 1 : 0;
    return val;
}

// Create a cons cell (pair)
NadaValue *nada_cons(NadaValue *car, NadaValue *cdr) {
    NadaValue *val = malloc(sizeof(NadaValue));
    val->type = NADA_PAIR;
    val->data.pair.car = car;
    val->data.pair.cdr = cdr;
    return val;
}

// Create a function value
NadaValue *nada_create_function(NadaValue *params, NadaValue *body, NadaEnv *env) {
    NadaValue *val = malloc(sizeof(NadaValue));
    val->type = NADA_FUNC;
    val->data.function.params = params;
    val->data.function.body = body;
    val->data.function.env = env;
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
    case NADA_STRING:
        free(val->data.string);
        break;
    case NADA_SYMBOL:
        free(val->data.symbol);
        break;
    case NADA_PAIR:
        nada_free(val->data.pair.car);
        nada_free(val->data.pair.cdr);
        break;
    case NADA_FUNC:
        nada_free(val->data.function.params);
        nada_free(val->data.function.body);
        // Don't free env - it might be shared
        break;
    default:
        // No special cleanup needed
        break;
    }

    free(val);
}

// Print a value (for debugging and REPL output)
void nada_print(NadaValue *val) {
    if (val == NULL) {
        printf("NULL");
        return;
    }

    switch (val->type) {
    case NADA_INT:
        printf("%d", val->data.integer);
        break;
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

    switch (val->type) {
    case NADA_INT:
        return nada_create_int(val->data.integer);

    case NADA_STRING:
        return nada_create_string(val->data.string);

    case NADA_SYMBOL:
        return nada_create_symbol(val->data.symbol);

    case NADA_NIL:
        return nada_create_nil();

    case NADA_PAIR: {
        // Recursively copy car and cdr
        NadaValue *new_car = nada_deep_copy(val->data.pair.car);
        NadaValue *new_cdr = nada_deep_copy(val->data.pair.cdr);
        return nada_cons(new_car, new_cdr);
    }

    case NADA_FUNC: {
        // Copy function including params, body and environment reference
        return nada_create_function(
            nada_deep_copy(val->data.function.params),
            nada_deep_copy(val->data.function.body),
            val->data.function.env  // Just copy the environment pointer
        );
    }

    case NADA_BOOL:
        return nada_create_bool(val->data.boolean);

    default:
        // Unknown type
        return nada_create_nil();
    }
}