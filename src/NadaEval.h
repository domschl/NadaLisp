#ifndef NADA_EVAL_H
#define NADA_EVAL_H

#include "NadaValue.h"

// Environment for storing bindings
typedef struct NadaEnv NadaEnv;

// Create a new environment
NadaEnv *nada_env_create(NadaEnv *parent);

// Free an environment and all its bindings
void nada_env_free(NadaEnv *env);

// Add a binding to the environment
void nada_env_set(NadaEnv *env, const char *name, NadaValue *value);

// Look up a binding in the environment
NadaValue *nada_env_get(NadaEnv *env, const char *name);

// Create a standard environment with basic operations
NadaEnv *nada_standard_env(void);

// Evaluate an expression in an environment
NadaValue *nada_eval(NadaValue *expr, NadaEnv *env);

#endif /* NADA_EVAL_H */