#ifndef NADA_ENV_H
#define NADA_ENV_H

#include "NadaValue.h"
#include "NadaEnv.h"

// Define the binding structure
struct NadaBinding {
    char *name;
    NadaValue *value;
    struct NadaBinding *next;
};

// Environment structure definition
struct NadaEnv {
    struct NadaBinding *bindings;
    struct NadaEnv *parent;
    int ref_count;
};

// Environment type
typedef struct NadaEnv NadaEnv;

// Environment lifecycle management functions
NadaEnv *nada_env_create(NadaEnv *parent);
// void nada_env_free(NadaEnv *env);

// Environment reference management functions
void nada_env_add_ref(NadaEnv *env);
void nada_env_break_all_cycles(NadaEnv *env);
void nada_env_force_free(NadaEnv *env);
void nada_env_release(NadaEnv *env);
void nada_cleanup_env(NadaEnv *global_env);
void nada_env_remove(NadaEnv *env, const char *name);

// Environment functions
void nada_env_set(NadaEnv *env, const char *name, NadaValue *value);
NadaValue *nada_env_get(NadaEnv *env, const char *name, int silent);
// Look up a symbol in the environment without printing error messages
NadaValue *nada_env_lookup_symbol(NadaEnv *env, const char *name);

#endif  // NADA_ENV_H
