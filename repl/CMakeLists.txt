# repl/CMakeLists.txt

# Create the executable
add_executable(nada NadaLisp.c)
target_link_libraries(nada PRIVATE nada_lib)

# Add include directories for the executable
target_include_directories(nada PRIVATE ${CMAKE_SOURCE_DIR}/lib)

# Add readline dependency to the executable
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
