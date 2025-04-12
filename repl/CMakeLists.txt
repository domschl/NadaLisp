# repl/CMakeLists.txt

# Link against the static library
add_executable(NadaLisp NadaLisp.c)
target_link_libraries(NadaLisp PRIVATE nada_lib)

# Add include directories for the executable
target_include_directories(NadaLisp PRIVATE ${CMAKE_SOURCE_DIR}/include)

# Add readline dependency to the executable
if(READLINE_FOUND)
    if(PkgConfig_FOUND AND READLINE_LIBRARIES)
        # When using pkg-config
        target_link_libraries(NadaLisp PRIVATE ${READLINE_LIBRARIES})
    else()
        # When found manually
        target_link_libraries(NadaLisp PRIVATE ${READLINE_LIBRARY})
        if(TERMCAP_LIBRARY)
            target_link_libraries(NadaLisp PRIVATE ${TERMCAP_LIBRARY})
        endif()
    endif()
    
    # Add include directories if needed
    if(READLINE_INCLUDE_DIR)
        target_include_directories(NadaLisp PRIVATE ${READLINE_INCLUDE_DIR})
    endif()
endif()

# Install the executable
install(TARGETS NadaLisp DESTINATION bin)
