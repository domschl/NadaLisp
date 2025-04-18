#ifndef NADA_STRING_H
#define NADA_STRING_H

#include "NadaValue.h"
#include "NadaEval.h"

// UTF-8 string utilities
int utf8_strlen(const char *str);
const char *utf8_index(const char *str, int index);
int utf8_charlen(const char *str);

// Convert a NadaValue to a string representation
char *nada_value_to_string(NadaValue *val);

// Exported string manipulation functions
NadaValue *builtin_string_length(NadaValue *args, NadaEnv *env);
NadaValue *builtin_substring(NadaValue *args, NadaEnv *env);
NadaValue *builtin_string_split(NadaValue *args, NadaEnv *env);
NadaValue *builtin_string_join(NadaValue *args, NadaEnv *env);
NadaValue *builtin_string_to_number(NadaValue *args, NadaEnv *env);
NadaValue *builtin_number_to_string(NadaValue *args, NadaEnv *env);
NadaValue *builtin_float(NadaValue *args, NadaEnv *env);
NadaValue *builtin_read_from_string(NadaValue *args, NadaEnv *env);
NadaValue *builtin_write_to_string(NadaValue *args, NadaEnv *env);

// Function to register all string functions
void register_string_functions(void);

#endif  // NADA_STRING_H