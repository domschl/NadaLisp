#ifndef NADA_PARSER_H
#define NADA_PARSER_H

#include "NadaValue.h"

// Tokenizer state
typedef struct {
    const char *input;
    size_t position;
    char token[1024];
} Tokenizer;

// Tokenizer functions
void tokenizer_init(Tokenizer *t, const char *input);
int get_next_token(Tokenizer *t);

// Parse a string into a NadaValue expression
NadaValue *nada_parse(const char *input);

// Parser functions
NadaValue *parse_expr(Tokenizer *t);

#endif /* NADA_PARSER_H */