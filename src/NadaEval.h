#ifndef NADA_EVAL_H
#define NADA_EVAL_H

#include "NadaValue.h"

// Environment type
typedef struct NadaEnv NadaEnv;

// Environment functions
NadaEnv *nada_env_create(NadaEnv *parent);
void nada_env_free(NadaEnv *env);
void nada_env_set(NadaEnv *env, const char *name, NadaValue *value);
NadaValue *nada_env_get(NadaEnv *env, const char *name);

// Create a standard environment with all built-in functions
NadaEnv *nada_create_standard_env(void);

// Evaluation function
NadaValue *nada_eval(NadaValue *expr, NadaEnv *env);

// Function to create a built-in function
NadaValue *nada_create_builtin_function(NadaValue *(*func)(NadaValue *, NadaEnv *));

// Add to public API:
NadaValue *nada_load_file(const char *filename, NadaEnv *env);

#endif  // NADA_EVAL_H