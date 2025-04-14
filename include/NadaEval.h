#ifndef NADA_EVAL_H
#define NADA_EVAL_H

#include <stdio.h>

#include "NadaEnv.h"
#include "NadaValue.h"

#include "NadaBuiltinLists.h"
#include "NadaBuiltinMath.h"
#include "NadaBuiltinCompare.h"
#include "NadaBuiltinSpecialForms.h"
#include "NadaBuiltinPredicates.h"
#include "NadaBuiltinBoolOps.h"
#include "NadaBuiltinIO.h"

// Type to represent a built-in function
typedef NadaValue *(*BuiltinFunc)(NadaValue *, NadaEnv *);

// Structure to hold built-in function info
typedef struct {
    const char *name;
    BuiltinFunc func;
} BuiltinFuncInfo;

// Hack to check for validity of a symbol without printing an error
void nada_set_silent_symbol_lookup(int silent);
bool nada_is_global_silent_symbol_lookup();

// Create a standard environment with all built-in functions
NadaEnv *nada_create_standard_env(void);

// Evaluation function
NadaValue *nada_eval(NadaValue *expr, NadaEnv *env);

// Function to create a built-in function
NadaValue *nada_create_builtin_function(NadaValue *(*func)(NadaValue *, NadaEnv *));

// Apply a function to arguments
NadaValue *apply_function(NadaValue *func, NadaValue *args, NadaEnv *env);
NadaValue *builtin_eval(NadaValue *args, NadaEnv *env);
NadaValue *builtin_tokenize_expr(NadaValue *args, NadaEnv *env);
NadaValue *builtin_string_to_symbol(NadaValue *args, NadaEnv *env);

// Check if a symbol is a built-in function and get its name
const char *get_builtin_name(BuiltinFunc func);

// Function to look up a built-in function by name
BuiltinFunc get_builtin_func(const char *name);

void nada_serialize_env(NadaEnv *current_env, FILE *out);

#endif  // NADA_EVAL_H