#ifndef NADABUILTINSPECIALFORMS_H
#define NADABUILTINSPECIALFORMS_H

#include "NadaValue.h"
#include "NadaError.h"

// Built-in function: quote
NadaValue *builtin_quote(NadaValue *args, NadaEnv *env);
// Built-in special form: define
NadaValue *builtin_define(NadaValue *args, NadaEnv *env);
// Built-in function: undef
NadaValue *builtin_undef(NadaValue *args, NadaEnv *env);
// Built-in special form: lambda
NadaValue *builtin_lambda(NadaValue *args, NadaEnv *env);
// Built-in special form: if
NadaValue *builtin_if(NadaValue *args, NadaEnv *env);
// Built-in special form: cond
NadaValue *builtin_cond(NadaValue *args, NadaEnv *env);
// Built-in special form: let (with support for named let)
NadaValue *builtin_let(NadaValue *args, NadaEnv *env);
// Built-in special form: begin
NadaValue *builtin_begin(NadaValue *args, NadaEnv *env);
// Built-in special form: set!
NadaValue *builtin_set(NadaValue *args, NadaEnv *env);

#endif  // NADABUILTINSPECIALFORMS_H
