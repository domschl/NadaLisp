# tests/CMakeLists.txt

# Create the test_runner library
add_library(test_runner STATIC test_runner.c)
target_link_libraries(test_runner PRIVATE nada_lib)

# Define individual test executables
add_executable(test_math test_math.c)
target_link_libraries(test_math PRIVATE test_runner)

add_executable(test_string test_string.c)
target_link_libraries(test_string PRIVATE test_runner)

add_executable(test_list test_list.c)
target_link_libraries(test_list PRIVATE test_runner)

add_executable(test_io test_io.c)
target_link_libraries(test_io PRIVATE test_runner)

# Add tests to CTest
add_test(NAME MathOperations COMMAND test_math)
add_test(NAME StringOperations COMMAND test_string)
add_test(NAME ListOperations COMMAND test_list)
add_test(NAME IOOperations COMMAND test_io)

# Lisp-based test runner
add_executable(run_lisp_tests run_lisp_tests.c)
target_link_libraries(run_lisp_tests PRIVATE test_runner nada_lib)

# Create lisp_tests directory if it doesn't exist
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lisp_tests)

# Add test to CTest
add_test(
    NAME LispBasedTests
    COMMAND run_lisp_tests ${CMAKE_CURRENT_SOURCE_DIR}/lisp_tests
)

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