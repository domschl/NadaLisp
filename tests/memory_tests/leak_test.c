// tests/memory_tests/leak_test.c
#include "../../src/NadaEval.h"
#include "../../src/NadaParser.h"
#include "../../src/NadaValue.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <script_file>\n", argv[0]);
        return 1;
    }

    // Open and read the test script
    FILE *file = fopen(argv[1], "r");
    if (!file) {
        fprintf(stderr, "Could not open file %s\n", argv[1]);
        return 1;
    }

    // Create a standard environment
    NadaEnv *env = nada_create_standard_env();

    // Read and evaluate each expression in the file
    char line[1024];
    while (fgets(line, sizeof(line), file)) {
        // Skip empty lines and comments
        if (line[0] == '\n' || line[0] == ';')
            continue;

        NadaValue *expr = nada_parse(line);
        NadaValue *result = nada_eval(expr, env);

        // Clean up
        nada_free(expr);
        nada_free(result);
    }

    // Clean up the environment
    nada_env_free(env);
    fclose(file);

    return 0;
}