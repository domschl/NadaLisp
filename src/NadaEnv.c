#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "NadaEnv.h"
#include "NadaEval.h"
#include "NadaError.h"

// Increment the reference count for an environment
void nada_env_add_ref(NadaEnv *env) {
    if (!env) return;
    env->ref_count++;
}

void nada_env_break_all_cycles(NadaEnv *env) {
    if (!env) return;

    // Break cycles in this environment
    struct NadaBinding *binding = env->bindings;
    while (binding) {
        if (binding->value && binding->value->type == NADA_FUNC) {
            // Set ALL function environments to NULL to break every possible cycle
            binding->value->data.function.env = NULL;
        }
        binding = binding->next;
    }

    // Recursively break cycles in parent environments
    if (env->parent) {
        nada_env_break_all_cycles(env->parent);
    }
}

// Decrement the reference count and free if zero
void nada_env_release(NadaEnv *env) {
    if (!env) return;

    // First, break any potential circular references
    struct NadaBinding *current = env->bindings;
    while (current) {
        if (current->value && current->value->type == NADA_FUNC) {
            // If this function references this exact environment, break the cycle
            if (current->value->data.function.env == env) {
                current->value->data.function.env = NULL;
            }
        }
        current = current->next;
    }

    // Now decrement reference count
    env->ref_count--;

    // Only free if reference count reaches zero
    if (env->ref_count > 0) {
        return;  // Still referenced elsewhere
    }

    // Free the environment's bindings
    current = env->bindings;
    while (current) {
        struct NadaBinding *next = current->next;
        nada_free(current->value);
        free(current->name);
        free(current);
        current = next;
    }

    // Release parent environment if it exists
    if (env->parent) {
        // env->parent->ref_count++;  // XXX corresponds to deactivation of parent env ref count in create
        nada_env_release(env->parent);
    }

    free(env);
}

void nada_env_force_free(NadaEnv *env) {
    if (!env) return;

    // First break all cycles
    nada_env_break_all_cycles(env);

    // Free all bindings regardless of reference count
    struct NadaBinding *current = env->bindings;
    while (current) {
        struct NadaBinding *next = current->next;
        if (current->value) {
            // If the value is a function, clear its env pointer to prevent
            // trying to access something we're about to free
            if (current->value->type == NADA_FUNC) {
                current->value->data.function.env = NULL;
            }
            nada_free(current->value);
        }
        free(current->name);
        free(current);
        current = next;
    }

    // Store parent so we can free it after this env is gone
    NadaEnv *parent = env->parent;

    // Free this environment
    free(env);

    // Free parent if it exists
    if (parent) {
        nada_env_force_free(parent);
    }
}

// Create a new environment
NadaEnv *nada_env_create(NadaEnv *parent) {
    NadaEnv *env = malloc(sizeof(NadaEnv));
    env->bindings = NULL;
    env->parent = parent;
    env->ref_count = 1;  // Start with ref count of 1

    // Add a reference to the parent if it exists
    if (parent) {
        nada_env_add_ref(parent);
    }
    return env;
}

// Add a binding to the environment
void nada_env_set(NadaEnv *env, const char *name, NadaValue *value) {
    // Check if symbol already exists
    struct NadaBinding *current = env->bindings;
    while (current != NULL) {
        if (strcmp(current->name, name) == 0) {
            // Free the old value before replacing it

            // Break circular references if the old value is a function
            // that refers to this environment
            if (current->value && current->value->type == NADA_FUNC &&
                current->value->data.function.env == env) {
                current->value->data.function.env = NULL;  // Break cycle before freeing
            }

            nada_free(current->value);

            // Store a copy of the value (so caller can free original)
            current->value = nada_deep_copy(value);

            // Special case for functions defined in this environment:
            // If we're storing a function that references this same environment,
            // decrement the reference count to avoid cycles
            if (current->value->type == NADA_FUNC &&
                current->value->data.function.env == env) {
                env->ref_count--;  // Cancel out the extra reference
            }

            return;
        }
        current = current->next;
    }

    // Add new binding
    struct NadaBinding *new_binding = malloc(sizeof(struct NadaBinding));
    new_binding->name = strdup(name);
    new_binding->value = nada_deep_copy(value);
    new_binding->next = env->bindings;  // Add to front of list
    env->bindings = new_binding;
}

// Look up a binding in the environment
NadaValue *nada_env_get(NadaEnv *env, const char *name, int silent) {
    // Search in current environment
    struct NadaBinding *current = env->bindings;
    while (current != NULL) {
        if (strcmp(current->name, name) == 0) {
            // Return a deep copy of the value, not the original!
            return nada_deep_copy(current->value);
        }
        current = current->next;
    }

    // If not found and we have a parent, search there
    if (env->parent != NULL) {
        return nada_env_get(env->parent, name, silent);
    }

    // Not found
    if (!silent && !nada_is_global_silent_symbol_lookup()) {
        nada_report_error(NADA_ERROR_UNDEFINED_SYMBOL,
                          "symbol '%s' not found in environment", name);
    }
    return nada_create_nil();  // Return nil for undefined symbols
}

// Remove a binding from the environment
void nada_env_remove(NadaEnv *env, const char *name) {
    struct NadaBinding *prev = NULL;
    struct NadaBinding *current = env->bindings;

    while (current != NULL) {
        if (strcmp(current->name, name) == 0) {
            // Found the binding to remove
            if (prev == NULL) {
                // It's the first binding
                env->bindings = current->next;
            } else {
                prev->next = current->next;
            }

            // Free the binding
            nada_free(current->value);
            free(current->name);
            free(current);
            return;
        }

        prev = current;
        current = current->next;
    }

    // If not found in current environment, try parent
    if (env->parent != NULL) {
        nada_env_remove(env->parent, name);
    }
}

// Look up a symbol in the environment without printing error messages
NadaValue *nada_env_lookup_symbol(NadaEnv *env, const char *name) {
    // Search in current environment
    struct NadaBinding *current = env->bindings;
    while (current != NULL) {
        if (strcmp(current->name, name) == 0) {
            // Return a deep copy of the value, not the original!
            return nada_deep_copy(current->value);
        }
        current = current->next;
    }

    // If not found and we have a parent, search there
    if (env->parent != NULL) {
        return nada_env_lookup_symbol(env->parent, name);
    }

    // Not found - return nil without reporting an error
    return nada_create_nil();
}

// Clean up the global environment
void nada_cleanup_env(NadaEnv *global_env) {
    if (global_env) {
        // Break circular references before releasing
        struct NadaBinding *binding = global_env->bindings;
        while (binding != NULL) {
            if (binding->value && binding->value->type == NADA_FUNC &&
                binding->value->data.function.env == global_env) {
                binding->value->data.function.env = NULL;
            }
            binding = binding->next;
        }

        // Now release the environment
        printf("Releasing global environment with ref count: %d\n", global_env->ref_count);
        nada_env_release(global_env);
        // nada_env_free(global_env);
        global_env = NULL;
    }
}
