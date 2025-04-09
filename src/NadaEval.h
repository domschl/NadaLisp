#ifndef NADA_EVAL_H
#define NADA_EVAL_H

#include "NadaValue.h"

// Environment type
typedef struct NadaEnv NadaEnv;

// Environment functions
NadaEnv *nada_env_create(NadaEnv *parent);
void nada_env_free(NadaEnv *env);
void nada_env_set(NadaEnv *env, const char *name, NadaValue *value);
NadaValue *nada_env_get(NadaEnv *env, const char *name, int silent);
// Look up a symbol in the environment without printing error messages
NadaValue *nada_env_lookup_symbol(NadaEnv *env, const char *name);

// Control silent symbol lookup
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

#endif  // NADA_EVAL_H