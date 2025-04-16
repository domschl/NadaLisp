#ifndef NADA_PARSER_H
#define NADA_PARSER_H

#include "NadaValue.h"
#include "NadaEnv.h"

// Tokenizer state
typedef struct {
    const char *input;
    size_t position;
    char token[1024];
} Tokenizer;

// Tokenizer functions
void tokenizer_init(Tokenizer *t, const char *input);
int get_next_token(Tokenizer *t);
int nada_validate_parentheses(const char *input, int *error_pos);

// Parse a string into a NadaValue expression
NadaValue *nada_parse(const char *input);
NadaValue *nada_parse_eval_multi(const char *input, NadaEnv *env);

// Parser functions
NadaValue *parse_expr(Tokenizer *t);

#endif /* NADA_PARSER_H */