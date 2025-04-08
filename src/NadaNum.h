#ifndef NADA_NUM_H
#define NADA_NUM_H

#include <stdbool.h>

// Forward declaration of the rational number type
typedef struct NadaNum NadaNum;

// Creation functions
NadaNum *nada_num_from_string(const char *str);
NadaNum *nada_num_from_int(int value);
NadaNum *nada_num_from_fraction(const char *numerator, const char *denominator);

// Memory management
NadaNum *nada_num_copy(const NadaNum *num);
void nada_num_free(NadaNum *num);

// Arithmetic operations
NadaNum *nada_num_add(const NadaNum *a, const NadaNum *b);
NadaNum *nada_num_subtract(const NadaNum *a, const NadaNum *b);
NadaNum *nada_num_multiply(const NadaNum *a, const NadaNum *b);
NadaNum *nada_num_divide(const NadaNum *a, const NadaNum *b);
NadaNum *nada_num_modulo(const NadaNum *a, const NadaNum *b);
NadaNum *nada_num_negate(const NadaNum *a);

// Comparison operations
bool nada_num_equal(const NadaNum *a, const NadaNum *b);
bool nada_num_less(const NadaNum *a, const NadaNum *b);
bool nada_num_greater(const NadaNum *a, const NadaNum *b);
bool nada_num_less_equal(const NadaNum *a, const NadaNum *b);
bool nada_num_greater_equal(const NadaNum *a, const NadaNum *b);

// Type checking
bool nada_num_is_integer(const NadaNum *num);
bool nada_num_is_zero(const NadaNum *num);
bool nada_num_is_positive(const NadaNum *num);
bool nada_num_is_negative(const NadaNum *num);

// Conversion functions
char *nada_num_to_string(const NadaNum *num);
int nada_num_to_int(const NadaNum *num);
double nada_num_to_double(const NadaNum *num);

// Parsing functions
bool nada_is_valid_number_string(const char *str);

#endif /* NADA_NUM_H */