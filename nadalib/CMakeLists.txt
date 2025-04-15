# CMakeLists.txt for nadalib

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
    NadaOutput.c
    NadaJupyter.c
)

# Add the library target
add_library(nada_lib STATIC ${LIB_SOURCES})
add_library(nada_shared SHARED ${LIB_SOURCES})

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Set the output directories for the library
set_target_properties(nada_lib PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set_target_properties(nada_shared PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Install include files
file(GLOB INCLUDE_FILES "${CMAKE_SOURCE_DIR}/include/*.h")

# Install the include files
install(FILES ${INCLUDE_FILES} DESTINATION include/nadalib)