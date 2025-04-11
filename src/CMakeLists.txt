# src/CMakeLists.txt

# Source files for the library
set(LIB_SOURCES
    NadaValue.c
    NadaEnv.c
    NadaParser.c
    NadaEval.c
    NadaString.c
    NadaNum.c
    NadaError.c
    NadaConfig.c
    NadaBuiltinLists.c
    NadaBuiltinMath.c
    NadaBuiltinCompare.c
    NadaBuiltinSpecialForms.c
    NadaBuiltinPredicates.c
    NadaBuiltinBoolOps.c
    NadaBuiltinIO.c
)

# Create the library with all the source code
add_library(nada_lib STATIC ${LIB_SOURCES})

# Set include directories for the library
target_include_directories(nada_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Create the executable
add_executable(nada NadaLisp.c)
target_link_libraries(nada PRIVATE nada_lib)

# Add readline dependency to the library - use variables from parent scope
if(READLINE_FOUND)
    # Add include directories
    target_include_directories(nada_lib PUBLIC ${READLINE_INCLUDE_DIR})
    
    # Add readline library
    target_link_libraries(nada_lib PUBLIC ${READLINE_LIBRARY})
    
    # Add termcap/ncurses if found
    if(TERMCAP_LIBRARY)
        target_link_libraries(nada_lib PUBLIC ${TERMCAP_LIBRARY})
    endif()
    
    # Define a preprocessor macro to enable readline functionality
    target_compile_definitions(nada_lib PUBLIC HAVE_READLINE)
endif()

if(READLINE_FOUND)
    if(PkgConfig_FOUND AND READLINE_LIBRARIES)
        # When using pkg-config
        target_link_libraries(nada PRIVATE ${READLINE_LIBRARIES})
    else()
        # When found manually
        target_link_libraries(nada PRIVATE ${READLINE_LIBRARY})
        if(TERMCAP_LIBRARY)
            target_link_libraries(nada PRIVATE ${TERMCAP_LIBRARY})
        endif()
    endif()
    
    # Add include directories if needed
    if(READLINE_INCLUDE_DIR)
        target_include_directories(nada PRIVATE ${READLINE_INCLUDE_DIR})
    endif()
endif()

# Install the executable
install(TARGETS nada DESTINATION bin)