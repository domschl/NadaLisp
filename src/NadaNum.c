#include "NadaNum.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

// Structure definition for a rational number
struct NadaNum {
    char *numerator;    // String representation of numerator digits
    char *denominator;  // String representation of denominator digits
    int sign;           // 1 for positive, -1 for negative
};

// Forward declarations of helper functions
static char *add_integers(const char *a, const char *b);
static char *subtract_integers(const char *a, const char *b);
static char *multiply_integers(const char *a, const char *b);
static char *divide_integers(const char *a, const char *b, char **remainder);
static int compare_integers(const char *a, const char *b);
static char *gcd(const char *a, const char *b);
static void normalize(NadaNum *num);
static char *strip_leading_zeros(const char *num);

// Create a rational number from an integer value
NadaNum *nada_num_from_int(int value) {
    NadaNum *num = malloc(sizeof(NadaNum));
    if (!num) return NULL;

    char buffer[32];
    sprintf(buffer, "%d", abs(value));

    num->numerator = strdup(buffer);
    num->denominator = strdup("1");
    num->sign = (value >= 0) ? 1 : -1;

    return num;
}

// Create a rational number from numerator and denominator strings
NadaNum *nada_num_from_fraction(const char *numerator, const char *denominator) {
    if (!numerator || !denominator) return NULL;

    // Check if denominator is zero
    bool denom_is_zero = true;
    for (const char *p = denominator; *p; p++) {
        if (*p != '0') {
            denom_is_zero = false;
            break;
        }
    }

    if (denom_is_zero) {
        fprintf(stderr, "Error: Division by zero\n");
        return nada_num_from_int(0);  // Return 0 instead of failing
    }

    NadaNum *num = malloc(sizeof(NadaNum));
    if (!num) return NULL;

    // Determine sign based on numerator (ignoring denominator sign)
    num->sign = 1;
    if (numerator[0] == '-') {
        num->sign = -1;
        numerator++;  // Skip the sign
    }

    // Skip denominator sign if present
    if (denominator[0] == '-') {
        num->sign *= -1;  // Flip sign
        denominator++;
    }

    // Copy values, skipping leading zeros
    num->numerator = strip_leading_zeros(numerator);
    if (!num->numerator || strcmp(num->numerator, "") == 0) {
        free(num->numerator);  // Add this to free any allocated memory
        num->numerator = strdup("0");
        num->sign = 1;  // Zero is always positive
    }

    num->denominator = strip_leading_zeros(denominator);
    if (!num->denominator || strcmp(num->denominator, "") == 0) {
        free(num->denominator);          // Add this to free any allocated memory
        num->denominator = strdup("1");  // Default denominator
    }

    // Normalize the fraction (reduce to lowest terms)
    normalize(num);

    return num;
}

// Parse a string into a rational number
NadaNum *nada_num_from_string(const char *str) {
    if (!str || *str == '\0') return NULL;

    // Copy string to work with
    char *s = strdup(str);
    if (!s) return NULL;

    // Determine the sign
    int sign = 1;
    char *p = s;

    if (*p == '+') {
        p++;
    } else if (*p == '-') {
        sign = -1;
        p++;
    }

    // Check for fraction notation (a/b)
    char *slash = strchr(p, '/');
    if (slash) {
        *slash = '\0';  // Split string at slash
        NadaNum *result = nada_num_from_fraction(p, slash + 1);
        if (result) {
            result->sign *= sign;
        }
        free(s);
        return result;
    }

    // Check for decimal notation
    char *dot = strchr(p, '.');
    if (dot) {
        *dot = '\0';  // Split at decimal point

        // Handle part before decimal
        char *integer_part = strip_leading_zeros(p);
        if (!integer_part || *integer_part == '\0') {
            integer_part = strdup("0");
        }

        // Convert decimal part to fraction
        char *decimal_part = strip_leading_zeros(dot + 1);
        size_t decimal_len = strlen(decimal_part);

        // Create denominator (10^decimal_len)
        char *denominator = malloc(decimal_len + 2);
        denominator[0] = '1';
        for (size_t i = 0; i < decimal_len; i++) {
            denominator[i + 1] = '0';
        }
        denominator[decimal_len + 1] = '\0';

        // Combine integer_part + decimal_part / 10^decimal_len
        char *combined_num = add_integers(
            multiply_integers(integer_part, denominator),
            decimal_part);

        NadaNum *result = nada_num_from_fraction(combined_num, denominator);
        if (result) {
            result->sign = sign;
        }

        free(integer_part);
        free(decimal_part);
        free(denominator);
        free(combined_num);
        free(s);

        return result;
    }

    // Simple integer
    NadaNum *result = nada_num_from_fraction(p, "1");
    if (result) {
        result->sign = sign;
    }

    free(s);
    return result;
}

// Create a copy of a rational number
NadaNum *nada_num_copy(const NadaNum *num) {
    if (!num) return NULL;

    NadaNum *copy = malloc(sizeof(NadaNum));
    if (!copy) return NULL;

    copy->numerator = strdup(num->numerator);
    copy->denominator = strdup(num->denominator);
    copy->sign = num->sign;

    return copy;
}

// Free a rational number
void nada_num_free(NadaNum *num) {
    if (!num) return;

    free(num->numerator);
    free(num->denominator);
    free(num);
}

// Add two rational numbers
NadaNum *nada_num_add(const NadaNum *a, const NadaNum *b) {
    if (!a || !b) return NULL;

    // Special case: if either operand is 0
    if (strcmp(a->numerator, "0") == 0) return nada_num_copy(b);
    if (strcmp(b->numerator, "0") == 0) return nada_num_copy(a);

    // a/b + c/d = (ad + bc)/bd
    char *ad = multiply_integers(a->numerator, b->denominator);
    char *bc = multiply_integers(b->numerator, a->denominator);
    char *bd = multiply_integers(a->denominator, b->denominator);

    if (!ad || !bc || !bd) {
        // Handle memory allocation failure
        free(ad);
        free(bc);
        free(bd);
        return nada_num_from_int(0);
    }

    // Apply signs
    char *numerator = NULL;
    int sign = 1;

    if (a->sign == b->sign) {
        // Same sign: add absolute values
        numerator = add_integers(ad, bc);
        sign = a->sign;
    } else {
        // Different signs: subtract the smaller from the larger
        int cmp = compare_integers(ad, bc);
        if (cmp >= 0) {
            numerator = subtract_integers(ad, bc);
            sign = a->sign;
        } else {
            numerator = subtract_integers(bc, ad);
            sign = b->sign;
        }
    }

    if (!numerator) {
        // Handle memory allocation failure
        free(ad);
        free(bc);
        free(bd);
        return nada_num_from_int(0);
    }

    // Create result
    NadaNum *result = malloc(sizeof(NadaNum));
    if (!result) {
        free(ad);
        free(bc);
        free(bd);
        free(numerator);
        return NULL;
    }

    result->numerator = strdup(numerator);
    result->denominator = strdup(bd);

    // Check for strdup failures
    if (!result->numerator || !result->denominator) {
        free(result->numerator);
        free(result->denominator);
        free(result);
        free(ad);
        free(bc);
        free(bd);
        free(numerator);
        return nada_num_from_int(0);
    }

    result->sign = sign;

    if (strcmp(result->numerator, "0") == 0) {
        result->sign = 1;  // Zero is always positive
    }

    // Normalize the fraction
    normalize(result);

    free(ad);
    free(bc);
    free(bd);
    free(numerator);

    return result;
}

// Subtract two rational numbers
NadaNum *nada_num_subtract(const NadaNum *a, const NadaNum *b) {
    if (!a || !b) return NULL;

    // a - b = a + (-b)
    NadaNum *neg_b = nada_num_copy(b);
    neg_b->sign = -neg_b->sign;

    NadaNum *result = nada_num_add(a, neg_b);

    nada_num_free(neg_b);
    return result;
}

// Multiply two rational numbers
NadaNum *nada_num_multiply(const NadaNum *a, const NadaNum *b) {
    if (!a || !b) return NULL;

    // Special case: if either operand is 0
    if (strcmp(a->numerator, "0") == 0 || strcmp(b->numerator, "0") == 0) {
        return nada_num_from_int(0);
    }

    // a/b * c/d = (a*c)/(b*d)
    char *ac = multiply_integers(a->numerator, b->numerator);
    char *bd = multiply_integers(a->denominator, b->denominator);

    NadaNum *result = nada_num_from_fraction(ac, bd);
    if (result) {
        result->sign = a->sign * b->sign;
    }

    free(ac);
    free(bd);

    return result;
}

// Divide two rational numbers
NadaNum *nada_num_divide(const NadaNum *a, const NadaNum *b) {
    if (!a || !b) return NULL;

    // Check for division by zero
    if (strcmp(b->numerator, "0") == 0) {
        fprintf(stderr, "Error: Division by zero\n");
        return nada_num_from_int(0);
    }

    // a/b / c/d = (a/b) * (d/c) = (a*d)/(b*c)
    char *ad = multiply_integers(a->numerator, b->denominator);
    char *bc = multiply_integers(a->denominator, b->numerator);

    NadaNum *result = nada_num_from_fraction(ad, bc);
    if (result) {
        result->sign = a->sign * b->sign;
    }

    free(ad);
    free(bc);

    return result;
}

// Calculate modulo of two rational numbers
NadaNum *nada_num_modulo(const NadaNum *a, const NadaNum *b) {
    if (!a || !b) return NULL;

    // Check for modulo by zero
    if (strcmp(b->numerator, "0") == 0) {
        fprintf(stderr, "Error: Modulo by zero\n");
        return nada_num_from_int(0);
    }

    // Only defined for integers
    if (!nada_num_is_integer(a) || !nada_num_is_integer(b)) {
        fprintf(stderr, "Error: Modulo only defined for integers\n");
        return nada_num_from_int(0);
    }

    char *remainder = NULL;
    divide_integers(a->numerator, b->numerator, &remainder);

    NadaNum *result = nada_num_from_fraction(remainder, "1");
    if (result) {
        result->sign = a->sign;
        if (strcmp(result->numerator, "0") == 0) {
            result->sign = 1;  // Zero is always positive
        }
    }

    free(remainder);

    return result;
}

// Negate a rational number
NadaNum *nada_num_negate(const NadaNum *a) {
    if (!a) return NULL;

    NadaNum *result = nada_num_copy(a);
    if (result) {
        result->sign = -result->sign;
        if (strcmp(result->numerator, "0") == 0) {
            result->sign = 1;  // Zero is always positive
        }
    }

    return result;
}

// Check if two rational numbers are equal
bool nada_num_equal(const NadaNum *a, const NadaNum *b) {
    if (!a || !b) return false;

    // After normalization, fractions are equal if their
    // numerators, denominators, and signs are identical
    return a->sign == b->sign &&
           strcmp(a->numerator, b->numerator) == 0 &&
           strcmp(a->denominator, b->denominator) == 0;
}

// Check if rational number a is less than b
bool nada_num_less(const NadaNum *a, const NadaNum *b) {
    if (!a || !b) return false;

    // Different signs
    if (a->sign < b->sign) return true;
    if (a->sign > b->sign) return false;

    // Same sign - compare a/b <? c/d by comparing a*d vs b*c
    char *ad = multiply_integers(a->numerator, b->denominator);
    char *bc = multiply_integers(b->numerator, a->denominator);

    int cmp = compare_integers(ad, bc);

    free(ad);
    free(bc);

    // If a->sign is positive, we want ad < bc
    // If a->sign is negative, we want ad > bc (larger negative is less)
    return (a->sign > 0) ? (cmp < 0) : (cmp > 0);
}

// Check if rational number a is greater than b
bool nada_num_greater(const NadaNum *a, const NadaNum *b) {
    return !nada_num_less(a, b) && !nada_num_equal(a, b);
}

// Check if rational number a is less than or equal to b
bool nada_num_less_equal(const NadaNum *a, const NadaNum *b) {
    return nada_num_less(a, b) || nada_num_equal(a, b);
}

// Check if rational number a is greater than or equal to b
bool nada_num_greater_equal(const NadaNum *a, const NadaNum *b) {
    return !nada_num_less(a, b);
}

// Check if a rational number is an integer
bool nada_num_is_integer(const NadaNum *num) {
    if (!num) return false;

    return strcmp(num->denominator, "1") == 0;
}

// Check if a rational number is zero
bool nada_num_is_zero(const NadaNum *num) {
    if (!num) return false;

    return strcmp(num->numerator, "0") == 0;
}

// Check if a rational number is positive
bool nada_num_is_positive(const NadaNum *num) {
    if (!num) return false;

    return num->sign > 0 && !nada_num_is_zero(num);
}

// Check if a rational number is negative
bool nada_num_is_negative(const NadaNum *num) {
    if (!num) return false;

    return num->sign < 0 && !nada_num_is_zero(num);
}

// Convert a rational number to a string
char *nada_num_to_string(const NadaNum *num) {
    if (!num) return NULL;

    // Special case for zero
    if (strcmp(num->numerator, "0") == 0) {
        return strdup("0");
    }

    // Integer case
    if (strcmp(num->denominator, "1") == 0) {
        char *result = malloc(strlen(num->numerator) + 2);  // +2 for sign and null terminator
        if (!result) return NULL;

        sprintf(result, "%s%s", (num->sign < 0 ? "-" : ""), num->numerator);
        return result;
    }

    // Fraction case
    char *result = malloc(strlen(num->numerator) + strlen(num->denominator) + 3);  // +3 for sign, slash, and null terminator
    if (!result) return NULL;

    sprintf(result, "%s%s/%s", (num->sign < 0 ? "-" : ""), num->numerator, num->denominator);
    return result;
}

// Convert a rational number to an integer
int nada_num_to_int(const NadaNum *num) {
    if (!num) return 0;

    // Calculate division
    char *remainder = NULL;
    char *quotient = divide_integers(num->numerator, num->denominator, &remainder);

    // Convert to integer
    int result = atoi(quotient) * num->sign;

    free(quotient);
    free(remainder);

    return result;
}

// Convert a rational number to a double
double nada_num_to_double(const NadaNum *num) {
    if (!num) return 0.0;

    // We could use arbitrary precision division here for better accuracy,
    // but for simplicity, we'll convert the strings to doubles and divide
    double numer = atof(num->numerator);
    double denom = atof(num->denominator);

    return (numer / denom) * num->sign;
}

// Check if a string is a valid number
bool nada_is_valid_number_string(const char *str) {
    if (!str || *str == '\0') return false;

    const char *p = str;

    // Check for sign
    if (*p == '+' || *p == '-') p++;

    // Need at least one digit
    if (!isdigit(*p) && *p != '.') return false;

    // Check for fraction
    bool has_slash = false;
    bool has_dot = false;

    while (*p) {
        if (*p == '/') {
            if (has_slash || has_dot) return false;  // Only one slash allowed, no slash after decimal
            has_slash = true;
        } else if (*p == '.') {
            if (has_dot || has_slash) return false;  // Only one dot allowed, no dot after slash
            has_dot = true;
        } else if (!isdigit(*p)) {
            return false;  // Only digits, dot, and slash allowed
        }
        p++;
    }

    // Can't end with slash or dot
    if (*(p - 1) == '/' || *(p - 1) == '.') return false;

    return true;
}

// Helper function implementations

// Add two arbitrary precision integers
static char *add_integers(const char *a, const char *b) {
    if (!a || !b) return NULL;

    // Determine the longer number
    size_t len_a = strlen(a);
    size_t len_b = strlen(b);
    size_t max_len = len_a > len_b ? len_a : len_b;

    // Allocate result with space for a possible carry
    char *result = malloc(max_len + 2);
    if (!result) return NULL;

    // Initialize the whole buffer with zeros to prevent issues
    memset(result, 0, max_len + 2);

    int carry = 0;
    size_t i = 0;

    // Add digits from right to left (least significant first)
    while (i < max_len || carry) {
        int digit_a = (i < len_a) ? a[len_a - 1 - i] - '0' : 0;
        int digit_b = (i < len_b) ? b[len_b - 1 - i] - '0' : 0;

        int sum = digit_a + digit_b + carry;
        carry = sum / 10;

        // Write digit from right to left
        result[max_len + 1 - i - 1] = (sum % 10) + '0';
        i++;
    }

    // Find where the number actually starts (skipping leading zeros)
    char *start = result;
    while (*start == 0 || *start == '0') {
        if (*(start + 1) == '\0') break;  // Keep at least one digit
        start++;
    }

    // Move the result to the beginning of the buffer
    char *final_result = strdup(start);
    free(result);

    return final_result;
}

// Subtract two arbitrary precision integers (assumes a >= b)
static char *subtract_integers(const char *a, const char *b) {
    if (!a || !b) return NULL;

    // If a < b, return "0" (this function assumes a >= b)
    if (compare_integers(a, b) < 0) {
        return strdup("0");
    }

    size_t len_a = strlen(a);
    size_t len_b = strlen(b);

    // Allocate result
    char *result = malloc(len_a + 1);
    if (!result) return NULL;

    int borrow = 0;
    size_t i = 0;

    // Subtract digits from right to left
    while (i < len_a) {
        int digit_a = a[len_a - 1 - i] - '0';
        int digit_b = (i < len_b) ? b[len_b - 1 - i] - '0' : 0;

        int diff = digit_a - digit_b - borrow;

        if (diff < 0) {
            diff += 10;
            borrow = 1;
        } else {
            borrow = 0;
        }

        result[len_a - 1 - i] = diff + '0';
        i++;
    }

    result[len_a] = '\0';

    // Remove leading zeros
    char *cleaned = strip_leading_zeros(result);
    free(result);

    // If result is empty, return "0"
    if (!cleaned || cleaned[0] == '\0') {
        free(cleaned);
        return strdup("0");
    }

    return cleaned;
}

// Multiply two arbitrary precision integers using the grade-school algorithm
static char *multiply_integers(const char *a, const char *b) {
    if (!a || !b) return NULL;

    // Special case for zero
    if (strcmp(a, "0") == 0 || strcmp(b, "0") == 0) {
        return strdup("0");
    }

    size_t len_a = strlen(a);
    size_t len_b = strlen(b);
    size_t result_len = len_a + len_b;

    // Allocate and initialize result array with zeros
    int *result_arr = calloc(result_len, sizeof(int));
    if (!result_arr) return NULL;

    // Multiply each digit of b with each digit of a
    for (size_t i = 0; i < len_a; i++) {
        for (size_t j = 0; j < len_b; j++) {
            int digit_a = a[len_a - 1 - i] - '0';
            int digit_b = b[len_b - 1 - j] - '0';

            result_arr[i + j] += digit_a * digit_b;
        }
    }

    // Handle carries
    for (size_t i = 0; i < result_len - 1; i++) {
        result_arr[i + 1] += result_arr[i] / 10;
        result_arr[i] %= 10;
    }

    // Convert result to string
    char *result_str = malloc(result_len + 1);
    if (!result_str) {
        free(result_arr);
        return NULL;
    }

    // Find the actual length (ignoring leading zeros)
    size_t actual_len = result_len;
    while (actual_len > 1 && result_arr[actual_len - 1] == 0) {
        actual_len--;
    }

    // Copy digits to result string (in reverse order)
    for (size_t i = 0; i < actual_len; i++) {
        result_str[actual_len - 1 - i] = result_arr[i] + '0';
    }
    result_str[actual_len] = '\0';

    free(result_arr);
    return result_str;
}

// Divide two arbitrary precision integers (long division algorithm)
static char *divide_integers(const char *a, const char *b, char **remainder) {
    if (!a || !b) {
        if (remainder) *remainder = strdup("0");
        return NULL;
    }

    // Check for division by zero
    if (strcmp(b, "0") == 0) {
        fprintf(stderr, "Error: Division by zero\n");
        if (remainder) *remainder = strdup("0");
        return strdup("0");
    }

    // If a < b, quotient is 0, remainder is a
    if (compare_integers(a, b) < 0) {
        if (remainder) *remainder = strdup(a);
        return strdup("0");
    }

    // If a == b, quotient is 1, remainder is 0
    if (compare_integers(a, b) == 0) {
        if (remainder) *remainder = strdup("0");
        return strdup("1");
    }

    // Long division algorithm
    size_t len_a = strlen(a);
    size_t len_b = strlen(b);
    size_t len_q = len_a - len_b + 1;  // Maximum possible quotient length

    char *quotient = malloc(len_q + 1);
    if (!quotient) {
        if (remainder) *remainder = strdup("0");
        return NULL;
    }

    // Initialize quotient with zeros
    memset(quotient, '0', len_q);
    quotient[len_q] = '\0';

    // Working copy of the dividend
    char *working = malloc(len_a + 1);
    if (!working) {
        free(quotient);
        if (remainder) *remainder = strdup("0");
        return NULL;
    }
    strcpy(working, a);

    // Process each digit of the quotient
    for (size_t i = 0; i < len_q; i++) {
        // Current divisor position
        size_t pos = i;

        // Try each digit (9 to 1)
        for (int digit = 9; digit >= 0; digit--) {
            // Set current quotient digit
            quotient[pos] = digit + '0';

            // Calculate product of partial quotient and divisor
            char partial_quotient[len_q + 1];
            memset(partial_quotient, '0', len_q);
            partial_quotient[pos] = digit + '0';
            partial_quotient[len_q] = '\0';

            char *product = multiply_integers(partial_quotient, b);

            // If product <= working substring, we found the digit
            if (product && compare_integers(product, working) <= 0) {
                // Subtract product from working
                char *new_working = subtract_integers(working, product);
                free(working);
                if (new_working) {
                    working = new_working;
                } else {
                    working = strdup("0");
                }

                free(product);
                break;
            }

            free(product);
        }
    }

    // Create remainder string without leading zeros
    if (remainder) {
        if (working[0] == '\0') {
            *remainder = strdup("0");
        } else {
            // Manually remove leading zeros to avoid leaks
            const char *p = working;
            while (*p == '0' && *(p + 1) != '\0')
                p++;
            *remainder = strdup(p);
        }
    }

    // Clean up quotient - manually remove leading zeros
    char *cleaned_quotient = NULL;
    const char *q = quotient;
    while (*q == '0' && *(q + 1) != '\0')
        q++;
    cleaned_quotient = strdup(q);

    free(quotient);
    free(working);

    return cleaned_quotient ? cleaned_quotient : strdup("0");
}

// Compare two arbitrary precision integers
static int compare_integers(const char *a, const char *b) {
    if (!a || !b) return 0;

    // Remove leading zeros
    while (*a == '0' && *(a + 1) != '\0')
        a++;
    while (*b == '0' && *(b + 1) != '\0')
        b++;

    // Different lengths
    size_t len_a = strlen(a);
    size_t len_b = strlen(b);

    if (len_a > len_b) return 1;
    if (len_a < len_b) return -1;

    // Same length, compare digit by digit
    return strcmp(a, b);
}

// Calculate greatest common divisor (GCD) using Euclidean algorithm
static char *gcd(const char *a, const char *b) {
    if (!a || !b) return strdup("1");  // Safe default

    // Base case: GCD(a, 0) = a
    if (strcmp(b, "0") == 0) {
        return strdup(a);
    }

    // Calculate a % b
    char *remainder = NULL;
    char *quotient = divide_integers(a, b, &remainder);
    free(quotient);  // We don't need the quotient

    if (!remainder) return strdup("1");  // Error fallback

    // Recursively calculate GCD(b, a % b)
    char *result = gcd(b, remainder);

    free(remainder);
    return result;
}

// Normalize a rational number (reduce to lowest terms)
static void normalize(NadaNum *num) {
    if (!num || !num->numerator || !num->denominator) return;

    // Special case: if numerator is 0, set denominator to 1
    if (strcmp(num->numerator, "0") == 0) {
        free(num->denominator);
        num->denominator = strdup("1");
        if (!num->denominator) {
            num->denominator = strdup("1");  // Try once more
        }
        num->sign = 1;  // Zero is always positive
        return;
    }

    // Calculate GCD of numerator and denominator
    char *g = gcd(num->numerator, num->denominator);
    if (!g) return;

    // If GCD is not 1, reduce the fraction
    if (strcmp(g, "1") != 0) {
        char *new_num = NULL;
        char *remainder = NULL;

        // numerator = numerator / gcd
        new_num = divide_integers(num->numerator, g, &remainder);
        if (new_num) {
            free(num->numerator);
            num->numerator = new_num;
        }
        if (remainder) free(remainder);

        // denominator = denominator / gcd
        new_num = divide_integers(num->denominator, g, &remainder);
        if (new_num) {
            free(num->denominator);
            num->denominator = new_num;
        }
        if (remainder) free(remainder);
    }

    free(g);
}

// Strip leading zeros from a number string
static char *strip_leading_zeros(const char *num) {
    if (!num) return strdup("0");

    // Skip leading zeros (but keep at least one digit)
    const char *p = num;
    while (*p == '0' && *(p + 1) != '\0')
        p++;

    return strdup(p);
}