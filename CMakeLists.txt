cmake_minimum_required(VERSION 3.15)

project(NadaLisp VERSION 0.1.0 LANGUAGES C)

# Set C standard to C17 (latest standard)
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Source files
set(SOURCES
    src/NadaLisp.c
    src/NadaValue.c
    src/NadaParser.c
    src/NadaEval.c
    src/NadaString.c
)

# Create executable
add_executable(nada ${SOURCES})

# Try to find readline using pkg-config first
find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
    pkg_check_modules(READLINE readline)
endif()

# If pkg-config didn't work, try to find readline manually (common on macOS with Homebrew)
if(NOT READLINE_FOUND)
    # Common Homebrew paths on macOS
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

# Link and include
if(READLINE_FOUND)
    if(TARGET READLINE::readline)
        target_link_libraries(nada PRIVATE READLINE::readline)
    else()
        target_include_directories(nada PRIVATE ${READLINE_INCLUDE_DIR})
        target_link_libraries(nada PRIVATE ${READLINE_LIBRARY})
        if(TERMCAP_LIBRARY)
            target_link_libraries(nada PRIVATE ${TERMCAP_LIBRARY})
        endif()
    endif()
else()
    message(FATAL_ERROR "readline library not found. Please install readline development package.")
endif()

# Installation
install(TARGETS nada DESTINATION bin)