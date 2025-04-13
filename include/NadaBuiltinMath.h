#ifndef __NADA_BUILTIN_MATH_H__
#define __NADA_BUILTIN_MATH_H__

#include "NadaValue.h"
#include "NadaEnv.h"

// Addition (+)
NadaValue *builtin_add(NadaValue *args, NadaEnv *env);
// Subtraction (-)
NadaValue *builtin_subtract(NadaValue *args, NadaEnv *env);
// Multiplication (*)
NadaValue *builtin_multiply(NadaValue *args, NadaEnv *env);
// Division (/)
NadaValue *builtin_divide(NadaValue *args, NadaEnv *env);
// Built-in function: modulo
NadaValue *builtin_modulo(NadaValue *args, NadaEnv *env);
NadaValue *builtin_remainder(NadaValue *args, NadaEnv *env);

#endif  // __NADA_BUILTIN_MATH_H__