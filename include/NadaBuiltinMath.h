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
// Exponentiation (expt)
NadaValue *builtin_expt(NadaValue *args, NadaEnv *env);

// Number component access functions
// Return the numerator of a rational number
NadaValue *builtin_numerator(NadaValue *args, NadaEnv *env);
// Return the denominator of a rational number
NadaValue *builtin_denominator(NadaValue *args, NadaEnv *env);
// Return the sign of a number (1 for positive, -1 for negative)
NadaValue *builtin_sign(NadaValue *args, NadaEnv *env);
// Return a list of prime factors of the numerator (if the number is an integer)
NadaValue *builtin_factor(NadaValue *args, NadaEnv *env);

#endif  // __NADA_BUILTIN_MATH_H__