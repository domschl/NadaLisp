# nadalib/CMakeLists.txt

# Install Scheme library files
file(GLOB SCHEME_LIB_FILES "*.scm")

# Copy library files to build directory
foreach(SCHEME_FILE ${SCHEME_LIB_FILES})
    get_filename_component(FILENAME ${SCHEME_FILE} NAME)
    configure_file(
        ${SCHEME_FILE}
        ${CMAKE_BINARY_DIR}/nadalib/${FILENAME}
        COPYONLY
    )
endforeach()

# Install the library files
install(FILES ${SCHEME_LIB_FILES} DESTINATION share/nadalib)
