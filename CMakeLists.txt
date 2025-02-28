# -----------------------------------------------------------------------------
# @brief  : Root cmake file.
# @author : Enrico Fraccaroli
# -----------------------------------------------------------------------------

# Set the minimum CMake version, the project name and default build type.
cmake_minimum_required(VERSION 3.1...3.18)

# Set the project name.
project(flexman CXX)

# Set the default build type to Debug.
if(NOT CMAKE_BUILD_TYPE)
    message(STATUS "Setting build type to 'Debug' as none was specified.")
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose the type of build." FORCE)
endif()

# -----------------------------------------------------------------------------
# ENABLE FETCH CONTENT
# -----------------------------------------------------------------------------

# We need this in order to import external projects.
include(FetchContent)
# Hide fetchcontent variables.
mark_as_advanced(FORCE
    FETCHCONTENT_QUIET
    FETCHCONTENT_BASE_DIR
    FETCHCONTENT_FULLY_DISCONNECTED
    FETCHCONTENT_UPDATES_DISCONNECTED
)

# -----------------------------------------------------------------------------
# OPTIONS
# -----------------------------------------------------------------------------

option(STRICT_WARNINGS "Enable strict compiler warnings" ON)
option(WARNINGS_AS_ERRORS "Treat all warnings as errors" OFF)

option(BUILD_EXAMPLES "Build examples" ON)

# -----------------------------------------------------------------------------
# DEPENDENCY (SYSTEM LIBRARIES)
# -----------------------------------------------------------------------------

# Find Doxygen.
find_package(Doxygen)

find_program(CLANG_TIDY_EXE NAMES clang-tidy)

# -----------------------------------------------------------------------------
# EXTERNAL DEPENDENCIES
# -----------------------------------------------------------------------------

FetchContent_Declare(
    json
    GIT_REPOSITORY "https://github.com/Galfurian/json.git"
    GIT_SHALLOW    TRUE
    GIT_PROGRESS   TRUE
    GIT_TAG        main
)
FetchContent_GetProperties(json)
if(NOT json_POPULATED)
    message(STATUS "Retrieving `json`...")
    FetchContent_MakeAvailable(json)
    # Hide fetchcontent variables.
    mark_as_advanced(FORCE FETCHCONTENT_UPDATES_DISCONNECTED_JSON FETCHCONTENT_SOURCE_DIR_JSON)
endif()

FetchContent_Declare(
    quire
    GIT_REPOSITORY "https://github.com/Galfurian/quire.git"
    GIT_SHALLOW    TRUE
    GIT_PROGRESS   TRUE
    GIT_TAG        main
)
FetchContent_GetProperties(quire)
if(NOT quire_POPULATED)
    message(STATUS "Retrieving `quire`...")
    FetchContent_MakeAvailable(quire)
    # Hide fetchcontent variables.
    mark_as_advanced(FORCE FETCHCONTENT_UPDATES_DISCONNECTED_QUIRE FETCHCONTENT_SOURCE_DIR_QUIRE)
endif()

FetchContent_Declare(
    timelib
    GIT_REPOSITORY "https://github.com/Galfurian/timelib.git"
    GIT_SHALLOW    TRUE
    GIT_PROGRESS   TRUE
    GIT_TAG        main
)
FetchContent_GetProperties(timelib)
if(NOT timelib_POPULATED)
    message(STATUS "Retrieving `timelib`...")
    FetchContent_MakeAvailable(timelib)
    # Hide fetchcontent variables.
    mark_as_advanced(FORCE 
        FETCHCONTENT_UPDATES_DISCONNECTED_TIMELIB
        FETCHCONTENT_SOURCE_DIR_TIMELIB
    )
endif()

# -----------------------------------------------------------------------------
# LIBRARY
# -----------------------------------------------------------------------------

# Add the C++ Library.
add_library(${PROJECT_NAME} INTERFACE)
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})
# Inlcude header directories.
target_include_directories(${PROJECT_NAME} INTERFACE
    ${PROJECT_SOURCE_DIR}/include
    ${json_SOURCE_DIR}/include
    ${quire_SOURCE_DIR}/include
    ${timelib_SOURCE_DIR}/include
)
# Link libraries.
target_link_libraries(${PROJECT_NAME} INTERFACE json quire timelib)
# Set the library to use c++-20
target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_20)

# -----------------------------------------------------------------------------
# COMPILATION FLAGS
# -----------------------------------------------------------------------------

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    # Disable warnings for MSVC-specific "safe" functions like strcpy_s, etc.,
    # which are not portable and may clutter warning logs.
    target_compile_definitions(${PROJECT_NAME} INTERFACE _CRT_SECURE_NO_WARNINGS)

    # Disable warning C4702: unreachable code.
    add_compile_options(/wd4702)

    if(WARNINGS_AS_ERRORS)
        # Treat all warnings as errors to enforce stricter code quality.
        target_compile_options(${PROJECT_NAME} INTERFACE /WX)
    endif()

    if(STRICT_WARNINGS)
        # Enable external header management to suppress warnings in system and
        # external headers, making it easier to focus on project-specific issues.
        target_compile_options(${PROJECT_NAME} INTERFACE /experimental:external)
        target_compile_options(${PROJECT_NAME} INTERFACE /external:I ${CMAKE_BINARY_DIR})
        target_compile_options(${PROJECT_NAME} INTERFACE /external:anglebrackets)
        target_compile_options(${PROJECT_NAME} INTERFACE /external:W0)

        # Use a high warning level to catch as many potential issues as possible.
        target_compile_options(${PROJECT_NAME} INTERFACE /W4)

        # Enforce standards-compliant behavior to avoid relying on MSVC-specific extensions.
        target_compile_options(${PROJECT_NAME} INTERFACE /permissive-)
    endif()

elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    if(WARNINGS_AS_ERRORS)
        # Treat all warnings as errors to enforce stricter code quality.
        target_compile_options(${PROJECT_NAME} INTERFACE -Werror)
    endif()

    if(STRICT_WARNINGS)
        # Enable a broad set of warnings to catch common and subtle issues:
        target_compile_options(${PROJECT_NAME} INTERFACE
            -Wall                # Enable most general-purpose warnings.
            -Wextra              # Enable extra warnings not included in -Wall.
            -Wconversion         # Warn about implicit type conversions that may lose data.
            -pedantic            # Enforce strict compliance with the C++ standard.
            -Wshadow             # Warn about variable shadowing, which can cause subtle bugs.
            -Wnon-virtual-dtor   # Warn when a class with virtual functions lacks a virtual destructor.
            -Wnull-dereference   # Warn about potential null pointer dereferences.
            -Wformat=2           # Enable strict checks for printf/scanf format strings.
            -Woverloaded-virtual # Warn when a derived class function hides a base class virtual function.
            -Wfloat-equal        # Warn about direct comparisons of floating-point values, which can be imprecise.
        )

    endif()
endif()

# -----------------------------------------------------------------------------
# EXAMPLES
# -----------------------------------------------------------------------------

if(BUILD_EXAMPLES)

    FetchContent_Declare(
        numint
        GIT_REPOSITORY "https://github.com/Galfurian/numint.git"
        GIT_SHALLOW    TRUE
        GIT_PROGRESS   TRUE
        GIT_TAG        main
    )
    FetchContent_GetProperties(numint)
    if(NOT numint_POPULATED)
        message(STATUS "Retrieving `numint`...")
        FetchContent_MakeAvailable(numint)
        # Hide fetchcontent variables.
        mark_as_advanced(FORCE FETCHCONTENT_UPDATES_DISCONNECTED_NUMINT FETCHCONTENT_SOURCE_DIR_NUMINT)
    endif()

    FetchContent_Declare(
        cmdlp
        GIT_REPOSITORY "https://github.com/Galfurian/cmdlp.git"
        GIT_SHALLOW    TRUE
        GIT_PROGRESS   TRUE
        GIT_TAG        main
    )
    FetchContent_GetProperties(cmdlp)
    if(NOT cmdlp_POPULATED)
        message(STATUS "Retrieving `cmdlp`...")
        FetchContent_MakeAvailable(cmdlp)
        # Hide fetchcontent variables.
        mark_as_advanced(FORCE FETCHCONTENT_UPDATES_DISCONNECTED_CMDLP FETCHCONTENT_SOURCE_DIR_CMDLP)
    endif()

    FetchContent_Declare(
        fsmlib
        GIT_REPOSITORY "https://github.com/Galfurian/fsmlib.git"
        GIT_SHALLOW    TRUE
        GIT_PROGRESS   TRUE
        GIT_TAG        main
    )
    FetchContent_GetProperties(fsmlib)
    if(NOT fsmlib_POPULATED)
        message(STATUS "Retrieving `fsmlib`...")
        FetchContent_MakeAvailable(fsmlib)
        # Hide fetchcontent variables.
        mark_as_advanced(FORCE FETCHCONTENT_UPDATES_DISCONNECTED_FSMLIB FETCHCONTENT_SOURCE_DIR_FSMLIB)
    endif()

    FetchContent_Declare(
        gpcpp
        GIT_REPOSITORY "https://github.com/Galfurian/gpcpp.git"
        GIT_SHALLOW    TRUE
        GIT_PROGRESS   TRUE
        GIT_TAG        main
    )
    FetchContent_GetProperties(gpcpp)
    if(NOT gpcpp_POPULATED)
        message(STATUS "Retrieving `gpcpp`...")
        FetchContent_MakeAvailable(gpcpp)
        # Hide fetchcontent variables.
        mark_as_advanced(FORCE FETCHCONTENT_UPDATES_DISCONNECTED_GPCPP FETCHCONTENT_SOURCE_DIR_GPCPP)
    endif()


    # Add the example.
    add_executable(${PROJECT_NAME}_tapping examples/tapping/main.cpp)
    target_include_directories(${PROJECT_NAME}_tapping PUBLIC
        ${PROJECT_SOURCE_DIR}/examples
        ${numint_SOURCE_DIR}/include
        ${fsmlib_SOURCE_DIR}/include
        ${cmdlp_SOURCE_DIR}/include
    )
    target_link_libraries(${PROJECT_NAME}_tapping PUBLIC
        ${PROJECT_NAME}
        numint
        fsmlib
        gpcpp
        cmdlp
    )

endif(BUILD_EXAMPLES)

# -----------------------------------------------------------------------------
# CODE ANALYSIS
# -----------------------------------------------------------------------------

if(CLANG_TIDY_EXE)
    file(GLOB_RECURSE PROJECT_HEADERS_AND_SOURCES
        "${PROJECT_SOURCE_DIR}/include/**/*.hpp"
        "${PROJECT_SOURCE_DIR}/include/**/*.hh"
        "${PROJECT_SOURCE_DIR}/include/**/*.h"
        "${PROJECT_SOURCE_DIR}/include/*.hpp"
        "${PROJECT_SOURCE_DIR}/include/*.hh"
        "${PROJECT_SOURCE_DIR}/include/*.h"
        "${PROJECT_SOURCE_DIR}/src/**/*.cpp"
        "${PROJECT_SOURCE_DIR}/src/**/*.cc"
        "${PROJECT_SOURCE_DIR}/src/**/*.c"
        "${PROJECT_SOURCE_DIR}/src/*.cpp"
        "${PROJECT_SOURCE_DIR}/src/*.cc"
        "${PROJECT_SOURCE_DIR}/src/*.c"
    )
    add_custom_target(
        ${PROJECT_NAME}_clang_tidy
        COMMAND ${CLANG_TIDY_EXE}
        --p=${CMAKE_BINARY_DIR}
        ${PROJECT_HEADERS_AND_SOURCES}
        COMMENT "Running clang-tidy"
        VERBATIM
    )
    add_custom_target(
        ${PROJECT_NAME}_clang_tidy_fix
        COMMAND ${CLANG_TIDY_EXE}
        --fix --fix-errors
        --p=${CMAKE_BINARY_DIR}
        ${PROJECT_HEADERS_AND_SOURCES}
        COMMENT "Running clang-tidy-fix"
        VERBATIM
    )
endif()

# -----------------------------------------------------------------------------
# DOCUMENTATION
# -----------------------------------------------------------------------------

if(DOXYGEN_FOUND)
    
    # FetchContent: Doxygen Awesome CSS
    FetchContent_Declare(doxygenawesome
        GIT_REPOSITORY https://github.com/jothepro/doxygen-awesome-css
        GIT_TAG main
    )
    FetchContent_MakeAvailable(doxygenawesome)

    # Hide FetchContent variables to avoid clutter in ccmake.
    mark_as_advanced(FORCE
        FETCHCONTENT_UPDATES_DISCONNECTED_DOXYGENAWESOME
        FETCHCONTENT_SOURCE_DIR_DOXYGENAWESOME
    )

    # Read the file with the version.
    file(READ ${PROJECT_SOURCE_DIR}/include/flexman/flexman.hpp version_file)
    # Extract the version.
    string(REGEX MATCH "FLEXMAN_MAJOR_VERSION ([0-9]*)" _ ${version_file})
    set(PROJECT_MAJOR_VERSION ${CMAKE_MATCH_1})
    string(REGEX MATCH "FLEXMAN_MINOR_VERSION ([0-9]*)" _ ${version_file})
    set(PROJECT_MINOR_VERSION ${CMAKE_MATCH_1})
    string(REGEX MATCH "FLEXMAN_MICRO_VERSION ([0-9]*)" _ ${version_file})
    set(PROJECT_MICRO_VERSION ${CMAKE_MATCH_1})

    # Customization: Doxygen Configuration
    set(DOXYGEN_WARN_FORMAT "$file:$line: $text")
    set(DOXYGEN_PROJECT_NAME FlexMan)
    set(DOXYGEN_PROJECT_BRIEF "Adaptive Scheduling and Optimization Library for C++")
    set(DOXYGEN_PROJECT_NUMBER "${PROJECT_MAJOR_VERSION}.${PROJECT_MINOR_VERSION}.${PROJECT_MICRO_VERSION}")
    set(DOXYGEN_USE_MDFILE_AS_MAINPAGE ${PROJECT_SOURCE_DIR}/README.md)
    set(DOXYGEN_SHOW_INCLUDE_FILES NO)
    set(DOXYGEN_GENERATE_TREEVIEW YES)
    set(DOXYGEN_GENERATE_LATEX NO)
    set(DOXYGEN_GENERATE_MAN NO)

    # Styling and UX enhancements using Doxygen Awesome
    set(DOXYGEN_HTML_HEADER ${doxygenawesome_SOURCE_DIR}/doxygen-custom/header.html)
    set(DOXYGEN_HTML_EXTRA_STYLESHEET ${doxygenawesome_SOURCE_DIR}/doxygen-awesome.css)
    set(DOXYGEN_HTML_EXTRA_FILES
        ${doxygenawesome_SOURCE_DIR}/doxygen-awesome-fragment-copy-button.js
        ${doxygenawesome_SOURCE_DIR}/doxygen-awesome-paragraph-link.js
        ${doxygenawesome_SOURCE_DIR}/doxygen-awesome-darkmode-toggle.js
    )

    # Set stricter warnings for better documentation quality
    set(DOXYGEN_WARN_IF_UNDOCUMENTED YES)
    set(DOXYGEN_WARN_IF_DOC_ERROR YES)
    set(DOXYGEN_WARN_NO_PARAMDOC YES)
    set(DOXYGEN_WARN_AS_ERROR YES)

    # Exclude certain files or directories from documentation (if needed)
    set(DOXYGEN_EXCLUDE_PATTERNS
        "${PROJECT_SOURCE_DIR}/tests/*"
        "${PROJECT_SOURCE_DIR}/examples/*"
    )

    file(GLOB_RECURSE PROJECT_HEADERS_AND_SOURCES
        "${PROJECT_SOURCE_DIR}/include/**/*.hpp"
        "${PROJECT_SOURCE_DIR}/include/**/*.hh"
        "${PROJECT_SOURCE_DIR}/include/**/*.h"
        "${PROJECT_SOURCE_DIR}/include/*.hpp"
        "${PROJECT_SOURCE_DIR}/include/*.hh"
        "${PROJECT_SOURCE_DIR}/include/*.h"
        "${PROJECT_SOURCE_DIR}/src/**/*.cpp"
        "${PROJECT_SOURCE_DIR}/src/**/*.cc"
        "${PROJECT_SOURCE_DIR}/src/**/*.c"
        "${PROJECT_SOURCE_DIR}/src/*.cpp"
        "${PROJECT_SOURCE_DIR}/src/*.cc"
        "${PROJECT_SOURCE_DIR}/src/*.c"
    )

    # Add Doxygen documentation target
    doxygen_add_docs(
        ${PROJECT_NAME}_documentation
        ${PROJECT_SOURCE_DIR}/README.md
        ${PROJECT_SOURCE_DIR}/LICENSE.md
        ${PROJECT_HEADERS_AND_SOURCES}
        COMMENT "Generating Doxygen documentation"
    )

endif()
