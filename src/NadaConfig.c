#include <unistd.h>
#include <dirent.h>

#include <stdio.h>
#include <string.h>

#include "NadaEnv.h"
#include "NadaValue.h"
#include "NadaConfig.h"
#include "NadaBuiltinIO.h"
#include "NadaError.h"

void nada_load_libraries(NadaEnv *env) {
    // Try multiple potential library locations
    const char *lib_dirs[] = {
        "src/nadalib",                // When run from project root
        "../src/nadalib",             // When run from build directory
        "../../src/nadalib",          // When run from build directory
        "./nadalib",                  // When run from src directory
        "/usr/local/share/nada/lib",  // System-wide installation
        NULL                          // End marker
    };

    DIR *dir = NULL;
    char cwd_buffer[1024];
    int found_index = -1;  // Store the index of the found directory

    // Get current working directory for better diagnostics
    if (getcwd(cwd_buffer, sizeof(cwd_buffer)) == NULL) {
        strcpy(cwd_buffer, "(unknown)");
    }

    printf("Searching for libraries from working directory: %s\n", cwd_buffer);

    // Try each potential location
    for (int i = 0; lib_dirs[i] != NULL; i++) {
        dir = opendir(lib_dirs[i]);
        if (dir) {
            printf("Found library directory: %s\n", lib_dirs[i]);
            found_index = i;  // Store the index
            break;
        }

        // Show the full path we tried
        if (lib_dirs[i][0] == '/') {
            // Absolute path
            printf("Tried library path: %s (not found)\n", lib_dirs[i]);
        } else {
            // Relative path
            printf("Tried library path: %s/%s (not found)\n", cwd_buffer, lib_dirs[i]);
        }
    }

    if (found_index < 0) {
        printf("Note: No library directory found. Libraries not loaded.\n");
        printf("Create the directory 'src/nadalib' and add .scm files there.\n");
        return;
    }

    // Use the actual found directory path for loading
    const char *found_dir = lib_dirs[found_index];
    printf("Loading libraries from %s...\n", found_dir);

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        // Skip directories and non-.scm files
        if (entry->d_type == DT_DIR) continue;

        const char *filename = entry->d_name;
        size_t len = strlen(filename);

        // Check for .scm extension
        if (len > 4 && strcmp(filename + len - 4, ".scm") == 0) {
            // Use the CORRECT found directory path
            char full_path[1024];
            snprintf(full_path, sizeof(full_path), "%s/%s", found_dir, filename);

            printf("  Loading %s\n", filename);
            NadaValue *result = nada_load_file(full_path, env);
            nada_free(result);
        }
    }

    closedir(dir);
    printf("Libraries loaded successfully.\n");
}
