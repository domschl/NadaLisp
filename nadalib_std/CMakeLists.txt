# Install Scheme library files
file(GLOB SCHEME_LIB_FILES "*.scm")

# Install include files
file(GLOB INCLUDE_FILES "${CMAKE_SOURCE_DIR}/include/*.h")

# Copy library files to build directory
foreach(SCHEME_FILE ${SCHEME_LIB_FILES})
    get_filename_component(FILENAME ${SCHEME_FILE} NAME)
    configure_file(
        ${SCHEME_FILE}
        ${CMAKE_BINARY_DIR}/nadalib_std/${FILENAME}
        COPYONLY
    )
endforeach()

# Copy include files to build directory
foreach(INCLUDE_FILE ${INCLUDE_FILES})
    get_filename_component(FILENAME ${INCLUDE_FILE} NAME)
    configure_file(
        ${INCLUDE_FILE}
        ${CMAKE_BINARY_DIR}/include/${FILENAME}
        COPYONLY
    )
endforeach()

# Install the library files
install(FILES ${SCHEME_LIB_FILES} DESTINATION share/nadalib)

# Install the include files
install(FILES ${INCLUDE_FILES} DESTINATION include/nadalib)
