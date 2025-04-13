#ifndef NADA_BUILTIN_PREDICATE_H
#define NADA_BUILTIN_PREDICATE_H

#include "NadaValue.h"
#include "NadaEnv.h"

// Empty list test (null?)
NadaValue *builtin_null(NadaValue *args, NadaEnv *env);
// Integer predicate (integer?)
NadaValue *builtin_integer_p(NadaValue *args, NadaEnv *env);
// Number predicate (number?)
NadaValue *builtin_number_p(NadaValue *args, NadaEnv *env);
// String predicate (string?)
NadaValue *builtin_string_p(NadaValue *args, NadaEnv *env);
// Symbol predicate (symbol?)
NadaValue *builtin_symbol_p(NadaValue *args, NadaEnv *env);
// Boolean predicate (boolean?)
NadaValue *builtin_boolean_p(NadaValue *args, NadaEnv *env);
// Pair predicate (pair?)
NadaValue *builtin_pair_p(NadaValue *args, NadaEnv *env);
// Function predicate (function?)
NadaValue *builtin_function_p(NadaValue *args, NadaEnv *env);
// Helper function to check if a value is a proper list
int is_proper_list(NadaValue *v);
// List predicate (list?)
NadaValue *builtin_list_p(NadaValue *args, NadaEnv *env);
// Atom predicate (atom?) - anything that's not a pair or nil
NadaValue *builtin_atom_p(NadaValue *args, NadaEnv *env);
// Error predicate (error?)
NadaValue *builtin_error_p(NadaValue *args, NadaEnv *env);

#endif  // NADA_BUILTIN_PREDICATE_H
