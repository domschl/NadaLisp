#ifndef NADA_VALUE_H
#define NADA_VALUE_H

#include <stddef.h>

// Forward declaration for NadaEnv (defined in NadaEval.h)
typedef struct NadaEnv NadaEnv;

// Type enumeration for our Lisp values
typedef enum {
    NADA_INT,     // Integer value
    NADA_STRING,  // String value
    NADA_SYMBOL,  // Symbol (identifier)
    NADA_PAIR,    // Cons cell (for lists)
    NADA_NIL,     // Empty list/nil
    NADA_FUNC,    // Function value
    NADA_BOOL     // Boolean value
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
    // Add this new field to store built-in function pointers
    NadaValue *(*builtin)(NadaValue *, struct NadaEnv *);
} NadaFunc;

// Main value structure (tagged union)
struct NadaValue {
    NadaValueType type;
    union {
        int integer;        // For NADA_INT
        char *string;       // For NADA_STRING
        char *symbol;       // For NADA_SYMBOL
        NadaPair pair;      // For NADA_PAIR
        NadaFunc function;  // For NADA_FUNC
        int boolean;        // For NADA_BOOL (1=true, 0=false)
        // NADA_NIL has no data
    } data;
};

// Constructor functions
NadaValue *nada_create_int(int value);
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

// Memory management
void nada_free(NadaValue *val);

// Deep copy a value
NadaValue *nada_deep_copy(NadaValue *val);

// Print function for debugging and REPL output
void nada_print(NadaValue *val);

// Memory report function
void nada_memory_report();
void nada_memory_reset();

// Memory tracking functions
void nada_increment_allocations(void);
void nada_increment_frees(void);

#endif /* NADA_VALUE_H */