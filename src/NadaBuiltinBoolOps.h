#ifndef NADA_BUILTIN_BOOL_OPS_H
#define NADA_BUILTIN_BOOL_OPS_H

#include "NadaValue.h"
#include "NadaError.h"

// Logical negation (not)
NadaValue *builtin_not(NadaValue *args, NadaEnv *env);
// Built-in special form: or
NadaValue *builtin_or(NadaValue *args, NadaEnv *env);
// Built-in special form: and
NadaValue *builtin_and(NadaValue *args, NadaEnv *env);

#endif  // NADA_BUILTIN_BOOL_OPS_H
