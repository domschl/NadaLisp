cmake_minimum_required(VERSION 3.15)

project(NadaLisp VERSION 0.1.0 LANGUAGES C)

# Set C standard to C17 (latest standard)
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Try to find readline using pkg-config first
find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
    pkg_check_modules(READLINE readline)
endif()

# If pkg-config didn't work, try to find readline manually (common on macOS with Homebrew)
if(NOT READLINE_FOUND)
    # Common Homebrew paths on macOS
    message("     Probably your are using Mac: searching mac paths...")
    find_path(READLINE_INCLUDE_DIR readline/readline.h
        PATHS
        /usr/local/opt/readline/include  # Intel Macs
        /opt/homebrew/opt/readline/include  # Apple Silicon Macs
        /usr/include
        /usr/local/include
    )
    
    find_library(READLINE_LIBRARY
        NAMES readline
        PATHS
        /usr/local/opt/readline/lib  # Intel Macs
        /opt/homebrew/opt/readline/lib  # Apple Silicon Macs
        /usr/lib
        /usr/local/lib
    )
    
    if(READLINE_INCLUDE_DIR AND READLINE_LIBRARY)
        set(READLINE_FOUND TRUE)
        # Also need to link against ncurses/termcap which readline depends on
        find_library(TERMCAP_LIBRARY
            NAMES termcap ncurses curses
            PATHS
            /usr/lib
            /usr/local/lib
            /usr/local/opt/ncurses/lib
            /opt/homebrew/opt/ncurses/lib
        )
    endif()
endif()

# Check if readline was found
if(NOT READLINE_FOUND)
    message(FATAL_ERROR "readline library not found. Please install readline development package.")
endif()

# Enable testing
enable_testing()

# Add the lib directory (core library)
add_subdirectory(nadalib)

# Add the repl directory (NadaLisp executable)
add_subdirectory(repl)

# Add the nadalib_std directory (Scheme standard library)
add_subdirectory(nadalib_std)

# Add the tests directory
add_subdirectory(tests)

# Add the Jupyter kernel directory
add_subdirectory(jupyter_kernel)

# Install library files
install(DIRECTORY ${CMAKE_SOURCE_DIR}/src/nadalib_std/
        DESTINATION ${CMAKE_INSTALL_PREFIX}/share/nada/lib
        FILES_MATCHING PATTERN "*.scm")

# Install the static and shared libraries
install(TARGETS nada_lib nada_shared
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib)

# Install the REPL executable
install(TARGETS nada DESTINATION bin)