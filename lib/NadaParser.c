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

// Update the next_token function to handle comments
static int next_token(Tokenizer *t) {
    // Skip whitespace
    while (isspace(t->input[t->position])) {
        t->position++;

        if (t->input[t->position] == '\0') {
            t->token[0] = '\0';
            return 0;
        }
    }

    // Handle comments (lines starting with ; or ;;)
    if (t->input[t->position] == ';') {
        // Skip to the end of the line
        while (t->input[t->position] != '\0' && t->input[t->position] != '\n') {
            t->position++;
        }

        // Skip the newline if present
        if (t->input[t->position] == '\n') {
            t->position++;
        }

        // Recursively call next_token to get the next valid token
        return next_token(t);
    }

    // Rest of the function remains the same
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

// Get the next token from the input
int get_next_token(Tokenizer *t) {
    return next_token(t);
}

// Update the parse_atom function to handle rational numbers
static NadaValue *parse_atom(Tokenizer *t) {
    // Check for boolean literals
    if (strcmp(t->token, "#t") == 0) {
        return nada_create_bool(1);
    } else if (strcmp(t->token, "#f") == 0) {
        return nada_create_bool(0);
    }

    // Check if it's a number
    if (nada_is_valid_number_string(t->token)) {
        // It's a valid number (integer, fraction or decimal)
        return nada_create_num_from_string(t->token);
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
            fprintf(stderr, "Error: unexpected end of input after quote\n");
            return nada_create_nil();
        }

        // Parse the quoted expression
        NadaValue *quoted_expr = parse_expr(t);

        // Create the quote symbol
        NadaValue *quote_sym = nada_create_symbol("quote");

        // Create the resulting expression: (quote <quoted_expr>)
        NadaValue *nil = nada_create_nil();
        NadaValue *inner = nada_cons(quoted_expr, nil);
        NadaValue *result = nada_cons(quote_sym, inner);

        // Free intermediate values that were deep-copied by nada_cons
        nada_free(quoted_expr);
        nada_free(quote_sym);
        nada_free(nil);
        nada_free(inner);

        return result;
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

    // Check for end of input after the first element
    if (t->token[0] == '\0') {
        fprintf(stderr, "Error: unterminated list, missing closing parenthesis\n");
        nada_free(head);
        return nada_create_nil();
    }

    // Handle dotted pairs
    if (strcmp(t->token, ".") == 0) {
        get_next_token(t);  // Consume the dot
        NadaValue *cdr = parse_expr(t);

        // Ensure the list is properly closed
        if (strcmp(t->token, ")") != 0) {
            fprintf(stderr, "Error: expected closing parenthesis after dotted pair\n");
            nada_free(head);
            nada_free(cdr);
            return nada_create_nil();
        }

        get_next_token(t);  // Consume closing parenthesis
        NadaValue *result = nada_cons(head, cdr);

        // Free the original values since nada_cons makes deep copies
        nada_free(head);
        nada_free(cdr);

        return result;
    }

    // Parse the rest of the list
    NadaValue *tail = parse_list(t);

    // Create cons cell and free the originals
    NadaValue *result = nada_cons(head, tail);
    nada_free(head);
    nada_free(tail);

    return result;
}

// Count and validate parentheses in a string
static int validate_parentheses(const char *input, int *error_pos) {
    int balance = 0;
    int in_string = 0;
    int in_comment = 0;
    int i = 0;

    while (input[i] != '\0') {
        // Handle comments
        if (input[i] == ';' && !in_string) {
            in_comment = 1;
        } else if (input[i] == '\n' && in_comment) {
            in_comment = 0;
        }

        // Only process characters outside of comments
        if (!in_comment) {
            // Handle strings
            if (input[i] == '"' && (i == 0 || input[i - 1] != '\\')) {
                in_string = !in_string;
            }

            // Only count parentheses outside of strings
            if (!in_string) {
                if (input[i] == '(') {
                    balance++;
                } else if (input[i] == ')') {
                    balance--;

                    // Detect too many closing parentheses
                    if (balance < 0) {
                        if (error_pos) *error_pos = i;
                        return -1;
                    }
                }
            }
        }
        i++;
    }

    // If we have unclosed parentheses, set error position to end of input
    if (balance > 0 && error_pos) {
        *error_pos = i - 1;
    }

    return balance;
}

// Parse from a string
NadaValue *nada_parse(const char *input) {
    // First validate parentheses
    int error_pos = -1;
    int paren_balance = validate_parentheses(input, &error_pos);

    if (paren_balance != 0) {
        fprintf(stderr, "Input: %s\n", input);
        if (paren_balance > 0) {
            fprintf(stderr, "Error: missing %d closing parentheses\n", paren_balance);
        } else {
            fprintf(stderr, "Error: unexpected closing parenthesis at position %d\n", error_pos);
        }

        // Show the context of the error
        if (error_pos >= 0) {
            int context_start = error_pos > 20 ? error_pos - 20 : 0;
            fprintf(stderr, "Context: %.*s\n", 40, input + context_start);

            // Print pointer to error position
            fprintf(stderr, "%*s^\n", error_pos - context_start, "");
            fprintf(stderr, "Full input:\n%s\n", input);
        }

        return nada_create_nil();
    }

    Tokenizer t;
    tokenizer_init(&t, input);

    // Get the first token
    if (!get_next_token(&t)) {
        // Empty input
        return nada_create_nil();
    }

    // Parse the expression
    NadaValue *result = parse_expr(&t);

    // Check if there's still more input
    if (t.token[0] != '\0' && strcmp(t.token, ")") != 0) {
        fprintf(stderr, "Warning: extra input after expression ignored\n");
    } else if (strcmp(t.token, ")") == 0) {
        fprintf(stderr, "Error: unexpected closing parenthesis\n");
    }

    return result;
}