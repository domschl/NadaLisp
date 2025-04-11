#ifndef NADA_BUILTIN_IO_H
#define NADA_BUILTIN_IO_H

#include "NadaValue.h"
#include "NadaEnv.h"

// Built-in function: save-environment
NadaValue *builtin_save_environment(NadaValue *args, NadaEnv *env);
// Built-in function: load-file
NadaValue *builtin_load_file(NadaValue *args, NadaEnv *env);
NadaValue *nada_load_file(const char *filename, NadaEnv *env);
// read-line: Read a line from console
NadaValue *builtin_read_line(NadaValue *args, NadaEnv *env);
// read-file: Read a file into a string
NadaValue *builtin_read_file(NadaValue *args, NadaEnv *env);
// write-file: Write a string to a file
NadaValue *builtin_write_file(NadaValue *args, NadaEnv *env);
// display: Output a string to console
NadaValue *builtin_display(NadaValue *args, NadaEnv *env);

#endif  // NADA_BUILTIN_IO_H
