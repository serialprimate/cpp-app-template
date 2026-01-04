cmake_minimum_required(VERSION 3.28.3)

project(
    cxxapp
    VERSION 0.1.0
    DESCRIPTION "Sample project."
    LANGUAGES CXX
)

# Include CTest
include(CTest)

# Require out-of-source builds
file(TO_CMAKE_PATH "${PROJECT_BINARY_DIR}/CMakeLists.txt" LOC_PATH)
if(EXISTS "${LOC_PATH}")
    message(
        FATAL_ERROR
            "You cannot build in a source directory (or any directory with a CMakeLists.txt file). Please make a build subdirectory. Feel free to remove CMakeCache.txt and CMakeFiles."
    )
endif()

# Set a default built type.
set(default_build_type "Release")
if(NOT (CMAKE_BUILD_TYPE OR CMAKE_CONFIGURATION_TYPES))
    message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
    set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE STRING "Choose the type of build." FORCE)
endif()

# Find: Do not use environment
set(CMAKE_FIND_USE_CMAKE_ENVIRONMENT_PATH FALSE)
set(CMAKE_FIND_USE_SYSTEM_ENVIRONMENT_PATH FALSE)

# Find: Use only local packages
set(CMAKE_FIND_USE_PACKAGE_REGISTRY FALSE)
set(CMAKE_FIND_USE_SYSTEM_PACKAGE_REGISTRY FALSE)

# Find: Do not use CMAKE_PREFIX_PATH
# set(CMAKE_FIND_USE_CMAKE_PATH FALSE)

# Find: Do not use CMAKE_SYSTEM_PREFIX_PATH
set(CMAKE_FIND_USE_CMAKE_SYSTEM_PATH FALSE)

# Ensure that for the following options that debug symbols are enabled.
# This is to ensure that the sanitizers can provide readable output.
# Also ensure that the sanitizers are NOT accidentally enabled for release builds.
if(NOT CMAKE_BUILD_TYPE MATCHES "^(Release|MinSizeRel)$")
    option(ENABLE_ASAN "Enable the address sanitiser globally" OFF)
    if(ENABLE_ASAN)
        add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
        add_link_options(-fsanitize=address)
    endif()

    option(ENABLE_UBSAN "Enable the undefined behaviour sanitiser globally" OFF)
    if(ENABLE_UBSAN)
        add_compile_options(-fsanitize=undefined -fno-omit-frame-pointer)
        add_link_options(-fsanitize=undefined)
    endif()

    option(ENABLE_TSAN "Enable the thread sanitiser globally" OFF)
    if(ENABLE_TSAN)
        add_compile_options(-fsanitize=thread -fPIE)
        add_link_options(-fsanitize=thread -pie)
    endif()
endif()

# Add library subdirectories
add_subdirectory(lib/logger)

# Add application subdirectories
add_subdirectory(app/sampleApp)

# Add test subdirectories (conditional)
if(BUILD_TESTING)
    add_subdirectory(test/unit/loggerUnitTest)
    add_subdirectory(test/int/appIntTest)
endif()

# Set the package name, version, description and vendor.
set(CPACK_PACKAGE_NAME "${PROJECT_NAME}")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${PROJECT_DESCRIPTION}")
set(CPACK_PACKAGE_VENDOR "serialprimate@gmail.com")

# Set the package file name to include version, architecture and build type.
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CMAKE_SYSTEM_PROCESSOR}-${CMAKE_BUILD_TYPE}")

# If the build type is Release or MinRelSize then strip the files.
if(CMAKE_BUILD_TYPE MATCHES "^(Release|MinRelSize)$")
    set(CPACK_STRIP_FILES TRUE)
endif()

# Includes the CPack module.
include(CPack)
