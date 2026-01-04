cmake_minimum_required(VERSION 3.28.3)

set(TARGET_NAME loggerUnitTest)

project(
    ${TARGET_NAME}
    VERSION 0.1.0
    DESCRIPTION "Unit tests for logger library."
    LANGUAGES CXX
)

# Add executable for unit tests
add_executable(${TARGET_NAME})

# Find GoogleTest
find_package(GTest CONFIG REQUIRED)

# Link to library-under-test and GoogleTest
target_link_libraries(
    ${TARGET_NAME}
    PRIVATE
        logger
        GTest::gtest
        GTest::gtest_main
)

# Specify source files
target_sources(
    ${TARGET_NAME}
    PRIVATE
        src/LoggerFactoryTest.cpp
)

# Set C++ standard
target_compile_features(
    ${TARGET_NAME}
    PRIVATE
        cxx_std_23
)

# Compiler warnings
target_compile_options(
    ${TARGET_NAME}
    PRIVATE
        $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:GNU>>:-Wall -Wextra -Wpedantic -Werror>
        $<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive- /WX>
)

# Set target properties
set_target_properties(
    ${TARGET_NAME}
    PROPERTIES
        CXX_EXTENSIONS FALSE
)

# Register with CTest
include(GoogleTest)
gtest_discover_tests(${TARGET_NAME})
