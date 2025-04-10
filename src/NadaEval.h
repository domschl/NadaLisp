#ifndef NADA_EVAL_H
#define NADA_EVAL_H

#include "NadaEnv.h"
#include "NadaValue.h"

// Type to represent a built-in function
typedef NadaValue *(*BuiltinFunc)(NadaValue *, NadaEnv *);

// Structure to hold built-in function info
typedef struct {
    const char *name;
    BuiltinFunc func;
} BuiltinFuncInfo;

void nada_set_silent_symbol_lookup(int silent);

// Create a standard environment with all built-in functions
NadaEnv *nada_create_standard_env(void);

// Evaluation function
NadaValue *nada_eval(NadaValue *expr, NadaEnv *env);

// Function to create a built-in function
NadaValue *nada_create_builtin_function(NadaValue *(*func)(NadaValue *, NadaEnv *));

// Apply a function to arguments
NadaValue *apply_function(NadaValue *func, NadaValue *args, NadaEnv *env);

// Add to public API:
NadaValue *nada_load_file(const char *filename, NadaEnv *env);

// Check if a symbol is a built-in function and get its name
const char *get_builtin_name(BuiltinFunc func);

// Load standard library files from available paths
// Returns the number of library files loaded
int nada_load_libraries(NadaEnv *env, int verbose);

#endif  // NADA_EVAL_H