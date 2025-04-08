#include "NadaParser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Initialize the tokenizer
void tokenizer_init(Tokenizer *t, const char *input) {
    t->input = input;
    t->position = 0;
    t->token[0] = '\0';
}

// Skip whitespace
static void skip_whitespace(Tokenizer *t) {
    while (t->input[t->position] != '\0' && isspace(t->input[t->position])) {
        t->position++;
    }
}

// Get the next token from the input
int get_next_token(Tokenizer *t) {
    skip_whitespace(t);

    if (t->input[t->position] == '\0') {
        t->token[0] = '\0';  // Explicitly set token to empty
        return 0;            // End of input
    }

    // Special characters (parentheses and quote)
    if (t->input[t->position] == '(' ||
        t->input[t->position] == ')' ||
        t->input[t->position] == '\'') {
        t->token[0] = t->input[t->position++];
        t->token[1] = '\0';
        return 1;
    }

    // String
    if (t->input[t->position] == '"') {
        size_t i = 0;
        t->token[i++] = t->input[t->position++];  // Add opening quote

        while (t->input[t->position] != '\0' && t->input[t->position] != '"') {
            // Handle escaped quotes
            if (t->input[t->position] == '\\' && t->input[t->position + 1] == '"') {
                t->token[i++] = '\\';
                t->position++;
            }
            t->token[i++] = t->input[t->position++];
        }

        if (t->input[t->position] == '"') {
            t->token[i++] = t->input[t->position++];  // Add closing quote
        }

        t->token[i] = '\0';
        return 1;
    }

    // Number or symbol
    size_t i = 0;
    while (t->input[t->position] != '\0' &&
           !isspace(t->input[t->position]) &&
           t->input[t->position] != '(' &&
           t->input[t->position] != ')') {
        t->token[i++] = t->input[t->position++];
    }
    t->token[i] = '\0';
    return 1;
}

// Parse an atom (number, string, symbol, boolean)
static NadaValue *parse_atom(Tokenizer *t) {
    // Check for boolean literals
    if (strcmp(t->token, "#t") == 0) {
        return nada_create_bool(1);
    } else if (strcmp(t->token, "#f") == 0) {
        return nada_create_bool(0);
    }

    // Check if it's a number
    char *endptr;
    long value = strtol(t->token, &endptr, 10);

    if (*endptr == '\0') {
        // It's a valid integer
        return nada_create_int((int)value);
    } else if (t->token[0] == '"' && t->token[strlen(t->token) - 1] == '"') {
        // It's a string - remove the quotes
        t->token[strlen(t->token) - 1] = '\0';
        return nada_create_string(t->token + 1);
    } else {
        // It's a symbol
        return nada_create_symbol(t->token);
    }
}

// Forward declaration for mutual recursion
static NadaValue *parse_list(Tokenizer *t);

// Parse an expression
NadaValue *parse_expr(Tokenizer *t) {
    // Check for empty input
    if (t->token[0] == '\0') {
        fprintf(stderr, "Error: unexpected end of input\n");
        return nada_create_nil();
    }

    // Check for quote shorthand ('x => (quote x))
    if (strcmp(t->token, "'") == 0) {
        // Get the next token
        if (!get_next_token(t)) {
            fprintf(stderr, "Error: quote (') without an expression\n");
            return nada_create_nil();
        }

        // Parse the quoted expression
        NadaValue *quoted_expr = parse_expr(t);

        // Create a symbol for 'quote'
        NadaValue *quote_sym = nada_create_symbol("quote");

        // Create the list (quote expr)
        NadaValue *quote_list = nada_cons(quote_sym, nada_cons(quoted_expr, nada_create_nil()));

        return quote_list;
    }

    // Handle regular expressions
    if (strcmp(t->token, "(") == 0) {
        // Move to the first token inside the list
        if (!get_next_token(t)) {
            fprintf(stderr, "Error: unterminated list, missing closing parenthesis\n");
            return nada_create_nil();
        }

        return parse_list(t);
    } else {
        NadaValue *result = parse_atom(t);
        get_next_token(t);  // Consume the atom token
        return result;
    }
}

// Parse a list (sequence of expressions inside parentheses)
static NadaValue *parse_list(Tokenizer *t) {
    // Check for empty list
    if (strcmp(t->token, ")") == 0) {
        get_next_token(t);  // Consume closing parenthesis
        return nada_create_nil();
    }

    // Check for end of input (unterminated list)
    if (t->token[0] == '\0') {
        fprintf(stderr, "Error: unterminated list, missing closing parenthesis\n");
        return nada_create_nil();
    }

    // Parse the first element
    NadaValue *head = parse_expr(t);

    // Check for end of input after the first element (this is the key fix)
    if (t->token[0] == '\0') {
        fprintf(stderr, "Error: unterminated list, missing closing parenthesis\n");
        nada_free(head);
        return nada_create_nil();
    }

    // Parse the rest of the list
    NadaValue *tail = parse_list(t);

    // Return a new pair
    return nada_cons(head, tail);
}

// Parse from a string
NadaValue *nada_parse(const char *input) {
    Tokenizer t;
    tokenizer_init(&t, input);

    // Get the first token
    if (!get_next_token(&t)) {
        // Empty input
        return nada_create_nil();
    }

    // Try-catch style parsing to catch errors
    int error_occurred = 0;
    NadaValue *result = NULL;

    // Parse the expression
    result = parse_expr(&t);

    // Special error handling for input like "(a"
    if (t.token[0] == '\0' && strchr(input, '(') != NULL && strchr(input, ')') == NULL) {
        fprintf(stderr, "Error: unterminated list, missing closing parenthesis\n");
        if (result != NULL) {
            nada_free(result);
        }
        return nada_create_nil();
    }

    // Check if there's still more input
    if (t.token[0] != '\0' && strcmp(t.token, ")") != 0) {
        fprintf(stderr, "Warning: extra input after expression ignored\n");
    } else if (strcmp(t.token, ")") == 0) {
        fprintf(stderr, "Error: unexpected closing parenthesis\n");
    }

    return result;
}