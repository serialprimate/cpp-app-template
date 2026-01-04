cmake_minimum_required(VERSION 3.28.3)

set(TARGET_NAME logger)

project(
    ${TARGET_NAME}
    VERSION 0.1.0
    DESCRIPTION "Sample logger library demonstrating interface+factory pattern."
    LANGUAGES CXX
)

# Add library target
add_library(${TARGET_NAME})

# Specify source files and public headers
target_sources(
    ${TARGET_NAME}
    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS
            ${CMAKE_CURRENT_SOURCE_DIR}/include
        FILES
            include/logger/ILogger.hpp
            include/logger/LoggerFactory.hpp
    PRIVATE
        src/ConsoleLogger.cpp
)

# Public include paths
target_include_directories(
    ${TARGET_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# Language standard requirement
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

# Target properties
set_target_properties(
    ${TARGET_NAME}
    PROPERTIES
        CXX_EXTENSIONS FALSE
)
