#ifndef NADABUILTINLISTS_H
#define NADABUILTINLISTS_H

#include "NadaValue.h"
#include "NadaEnv.h"

// Built-in function: car
NadaValue *builtin_car(NadaValue *args, NadaEnv *env);
// Built-in function: cdr
NadaValue *builtin_cdr(NadaValue *args, NadaEnv *env);
// cadr: Get the second element of a list (car of cdr)
NadaValue *builtin_cadr(NadaValue *args, NadaEnv *env);
// caddr: Get the third element of a list (car of cdr of cdr)
NadaValue *builtin_caddr(NadaValue *args, NadaEnv *env);
// sublist: Extract a portion of a list
NadaValue *builtin_sublist(NadaValue *args, NadaEnv *env);
// list-ref: Get an element at a specific position in a list
NadaValue *builtin_list_ref(NadaValue *args, NadaEnv *env);
// Map function: Apply a function to each element of a list
NadaValue *builtin_map(NadaValue *args, NadaEnv *env);
// Cons function: Create a pair
NadaValue *builtin_cons(NadaValue *args, NadaEnv *env);
// Fix in NadaEval.c
NadaValue *builtin_list(NadaValue *args, NadaEnv *env);
// Length function - count elements in a list
NadaValue *builtin_length(NadaValue *args, NadaEnv *env);

#endif