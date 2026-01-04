cmake_minimum_required(VERSION 3.28.3)

set(TARGET_NAME sampleApp)

project(
    ${TARGET_NAME}
    VERSION 0.1.0
    DESCRIPTION "Sample application demonstrating composition root pattern."
    LANGUAGES CXX
)

# Add executable target
add_executable(${TARGET_NAME})

# Specify source files
target_sources(
    ${TARGET_NAME}
    PRIVATE
        src/main.cpp
)

# Link to logger library
target_link_libraries(
    ${TARGET_NAME}
    PRIVATE
        logger
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
