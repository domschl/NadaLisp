#ifndef NADA_VALUE_H
#define NADA_VALUE_H

#include <stddef.h>
#include "NadaNum.h"  // Include the new NadaNum header

// Forward declaration for NadaEnv (defined in NadaEval.h)
typedef struct NadaEnv NadaEnv;

// Type enumeration for our Lisp values
typedef enum {
    NADA_NUM,     // Number
    NADA_STRING,  // String value
    NADA_SYMBOL,  // Symbol (identifier)
    NADA_PAIR,    // Cons cell (for lists)
    NADA_NIL,     // Empty list/nil
    NADA_FUNC,    // Function value
    NADA_BOOL,    // Boolean value
    NADA_ERROR    // Error value
} NadaValueType;

// Forward declaration
typedef struct NadaValue NadaValue;

// Pair structure (cons cell)
typedef struct {
    NadaValue *car;  // First element (head)
    NadaValue *cdr;  // Rest of the list (tail)
} NadaPair;

// Function structure
typedef struct {
    NadaValue *params;    // Parameter list
    NadaValue *body;      // Function body
    struct NadaEnv *env;  // Captured environment (closure)
    NadaValue *(*builtin)(NadaValue *, struct NadaEnv *);
} NadaFunc;

// Main value structure (tagged union)
struct NadaValue {
    NadaValueType type;
    union {
        NadaNum *number;    // For NADA_NUM
        char *string;       // For NADA_STRING
        char *error;        // For NADA_ERROR
        char *symbol;       // For NADA_SYMBOL
        NadaPair pair;      // For NADA_PAIR
        NadaFunc function;  // For NADA_FUNC
        int boolean;        // For NADA_BOOL (1=true, 0=false)
        // NADA_NIL has no data
    } data;
};

// Constructor functions
NadaValue *nada_create_num(NadaNum *num);                 // New function
NadaValue *nada_create_num_from_int(int value);           // Replacement for nada_create_int
NadaValue *nada_create_num_from_string(const char *str);  // New function
NadaValue *nada_create_string(const char *str);
NadaValue *nada_create_symbol(const char *name);
NadaValue *nada_create_nil(void);
NadaValue *nada_cons(NadaValue *car, NadaValue *cdr);
NadaValue *nada_create_function(NadaValue *params, NadaValue *body, NadaEnv *env);
NadaValue *nada_create_bool(int boolean);

// List operations
NadaValue *nada_car(NadaValue *pair);
NadaValue *nada_cdr(NadaValue *pair);
int nada_is_nil(NadaValue *val);
NadaValue *nada_reverse(NadaValue *list);

// Error handling: functions for creating and checking errors:
NadaValue *nada_create_error(const char *message);
int nada_is_error(const NadaValue *value);

// Memory management
void nada_free(NadaValue *val);

// Deep copy a value
NadaValue *nada_deep_copy(NadaValue *val);

// Type to string
const char *nada_type_name(int type);

// Print function for debugging and REPL output
void nada_print(NadaValue *val);

// Memory report function
void nada_memory_report();
void nada_memory_reset();

// Memory tracking functions
void nada_increment_allocations(void);
void nada_increment_frees(void);

#endif /* NADA_VALUE_H */