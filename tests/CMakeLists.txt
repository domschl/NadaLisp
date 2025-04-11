# tests/CMakeLists.txt

# Lisp-based test runner
add_executable(run_lisp_tests run_lisp_tests.c)
target_link_libraries(run_lisp_tests PRIVATE nada_lib)

# Copy Lisp test files to build directory
file(GLOB LISP_TEST_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/lisp_tests/*.scm"
)

# Create lisp_tests directory in build directory
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lisp_tests)

# Copy each test file
foreach(LISP_TEST_FILE ${LISP_TEST_FILES})
    get_filename_component(FILENAME ${LISP_TEST_FILE} NAME)
    configure_file(
        ${LISP_TEST_FILE}
        ${CMAKE_CURRENT_BINARY_DIR}/lisp_tests/${FILENAME}
        COPYONLY
    )
endforeach()

# Remove existing LispBasedTests definition
if(TARGET LispBasedTests)
  remove_test(LispBasedTests)
endif()

# Add updated test definition
message("Adding test")
add_test(
  NAME LispBasedTests
  COMMAND run_lisp_tests "${CMAKE_CURRENT_SOURCE_DIR}/lisp_tests"
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/tests
)
message("End add test")

# Memory leak test
if(UNIX)
    # Create script for running memory tests
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/memory_tests/leak_test.sh.in
        ${CMAKE_CURRENT_BINARY_DIR}/leak_test.sh
        @ONLY
    )
    
    file(CHMOD ${CMAKE_CURRENT_BINARY_DIR}/leak_test.sh
         PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE 
                    GROUP_READ GROUP_EXECUTE 
                    WORLD_READ WORLD_EXECUTE)
    
    add_executable(memory_test memory_tests/leak_test.c)
    target_link_libraries(memory_test PRIVATE nada_lib)
    
    # Add memory leak tests based on platform
    if(APPLE)
        add_test(
            NAME MemoryLeakCheck
            COMMAND ${CMAKE_CURRENT_BINARY_DIR}/leak_test.sh leaks
        )
    else()
        # Linux (need to check if valgrind is available)
        find_program(VALGRIND "valgrind")
        if(VALGRIND)
            add_test(
                NAME MemoryLeakCheck
                COMMAND ${CMAKE_CURRENT_BINARY_DIR}/leak_test.sh valgrind
            )
        endif()
    endif()
endif()
