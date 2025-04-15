#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "NadaValue.h"
#include "NadaEval.h"
#include "NadaOutput.h"
#include "NadaError.h"

// Output markdown in Jupyter
NadaValue *builtin_display_markdown(NadaValue *args, NadaEnv *env) {
    if (nada_is_nil(args)) {
        return nada_create_error("display-markdown requires at least 1 argument");
    }

    // Set output type to markdown
    nada_jupyter_clear_buffer();  // Clear any previous output
    nada_jupyter_set_output_type(NADA_OUTPUT_MARKDOWN);

    // Process all arguments
    NadaValue *curr = args;
    while (!nada_is_nil(curr)) {
        NadaValue *val = nada_eval(nada_car(curr), env);

        // Convert to string if not already
        if (val->type != NADA_STRING) {
            char *str = nada_value_to_string(val);
            nada_write_string(str);
            free(str);
        } else {
            nada_write_string(val->data.string);
        }

        nada_free(val);
        curr = nada_cdr(curr);

        // Add space between arguments
        if (!nada_is_nil(curr)) {
            nada_write_string(" ");
        }
    }

    return nada_create_nil();
}

// Output HTML in Jupyter
NadaValue *builtin_display_html(NadaValue *args, NadaEnv *env) {
    if (nada_is_nil(args)) {
        return nada_create_error("display-html requires at least 1 argument");
    }

    // Set output type to HTML
    nada_jupyter_clear_buffer();  // Clear any previous output
    nada_jupyter_set_output_type(NADA_OUTPUT_HTML);

    // Process all arguments
    NadaValue *curr = args;
    while (!nada_is_nil(curr)) {
        NadaValue *val = nada_eval(nada_car(curr), env);

        // Convert to string if not already
        if (val->type != NADA_STRING) {
            char *str = nada_value_to_string(val);
            nada_write_string(str);
            free(str);
        } else {
            nada_write_string(val->data.string);
        }

        nada_free(val);
        curr = nada_cdr(curr);

        // Add space between arguments
        if (!nada_is_nil(curr)) {
            nada_write_string(" ");
        }
    }

    return nada_create_nil();
}