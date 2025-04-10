#ifndef NADA_BUILTIN_COMPARE_H
#define NADA_BUILTIN_COMPARE_H

#include <string.h>

#include "NadaValue.h"
#include "NadaEval.h"
#include "NadaError.h"

// Less than (<)
NadaValue *builtin_less_than(NadaValue *args, NadaEnv *env);
// Less than or equal (<=)
NadaValue *builtin_less_equal(NadaValue *args, NadaEnv *env);
// Greater than (>)
NadaValue *builtin_greater_than(NadaValue *args, NadaEnv *env);
// Greater than or equal (>=)
NadaValue *builtin_greater_equal(NadaValue *args, NadaEnv *env);
// Numeric equality (=)
NadaValue *builtin_numeric_equal(NadaValue *args, NadaEnv *env);
// Identity equality (eq?)
NadaValue *builtin_eq(NadaValue *args, NadaEnv *env);
// Helper function for recursive equality check
int values_equal(NadaValue *a, NadaValue *b);
// Recursive structural equality (equal?)
NadaValue *builtin_equal(NadaValue *args, NadaEnv *env);

#endif  // NADA_BUILTIN_COMPARE_H