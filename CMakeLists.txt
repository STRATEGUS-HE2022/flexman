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

FetchContent_Declare(
    chainsaw
    GIT_REPOSITORY "https://github.com/Galfurian/chainsaw.git"
    GIT_SHALLOW    TRUE
    GIT_PROGRESS   TRUE
    GIT_TAG        main
)
FetchContent_GetProperties(chainsaw)
if(NOT chainsaw_POPULATED)
    message(STATUS "Retrieving `chainsaw`...")
    FetchContent_MakeAvailable(chainsaw)
    # Hide fetchcontent variables.
    mark_as_advanced(FORCE FETCHCONTENT_UPDATES_DISCONNECTED_CHAINSAW FETCHCONTENT_SOURCE_DIR_CHAINSAW)
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
    malg
    GIT_REPOSITORY "https://github.com/Galfurian/malg.git"
    GIT_SHALLOW    TRUE
    GIT_PROGRESS   TRUE
    GIT_TAG        main
)
FetchContent_GetProperties(malg)
if(NOT malg_POPULATED)
    message(STATUS "Retrieving `malg`...")
    FetchContent_MakeAvailable(malg)
    # Hide fetchcontent variables.
    mark_as_advanced(FORCE FETCHCONTENT_UPDATES_DISCONNECTED_MALG FETCHCONTENT_SOURCE_DIR_MALG)
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

# -----------------------------------------------------------------------------
# LIBRARY
# -----------------------------------------------------------------------------

# Add the C++ Library.
add_library(flexman INTERFACE)
add_library(flexman::flexman ALIAS flexman)
# Inlcude header directories.
target_include_directories(flexman INTERFACE
    ${PROJECT_SOURCE_DIR}/include
    ${json_SOURCE_DIR}/include
    ${quire_SOURCE_DIR}/include
    ${timelib_SOURCE_DIR}/include
)
# Link libraries.
target_link_libraries(flexman INTERFACE json quire timelib)
# Set the library to use c++-20
target_compile_features(flexman INTERFACE cxx_std_20)

# -----------------------------------------------------------------------------
# COMPILATION FLAGS
# -----------------------------------------------------------------------------

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    # Disable warnings that suggest using MSVC-specific safe functions
    target_compile_definitions(flexman INTERFACE _CRT_SECURE_NO_WARNINGS)
    if(WARNINGS_AS_ERRORS)
        target_compile_options(flexman INTERFACE /WX)
    endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    if(WARNINGS_AS_ERRORS)
        target_compile_options(flexman INTERFACE -Werror)
    endif()
endif()

if(STRICT_WARNINGS)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        # Mark system headers as external for MSVC explicitly
        # https://devblogs.microsoft.com/cppblog/broken-warnings-theory
        target_compile_options(flexman INTERFACE /experimental:external)
        target_compile_options(flexman INTERFACE /external:I ${CMAKE_BINARY_DIR})
        target_compile_options(flexman INTERFACE /external:anglebrackets)
        target_compile_options(flexman INTERFACE /external:W0)

        target_compile_options(flexman INTERFACE /W4)
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        target_compile_options(flexman INTERFACE -Wall -Wextra -Wconversion -pedantic)
    endif()
endif()

# -----------------------------------------------------------------------------
# EXAMPLES
# -----------------------------------------------------------------------------

if(BUILD_EXAMPLES)
    
    # Add the example.
    add_executable(flexman_tapping examples/tapping/main.cpp)
    target_include_directories(flexman_tapping PUBLIC
        ${PROJECT_SOURCE_DIR}/examples
        ${chainsaw_SOURCE_DIR}/include
        ${fsmlib_SOURCE_DIR}/include
        ${cmdlp_SOURCE_DIR}/include
        ${malg_SOURCE_DIR}/include
    )
    target_link_libraries(flexman_tapping PUBLIC
        chainsaw
        flexman
        fsmlib
        gpcpp
        cmdlp
        malg
    )

endif(BUILD_EXAMPLES)


# -----------------------------------------------------------------------------
# DOCUMENTATION
# -----------------------------------------------------------------------------

if(DOXYGEN_FOUND)
    
    # Record the options that describe how to populate the specified content.
    FetchContent_Declare(
        doxygenawesome
        GIT_REPOSITORY https://github.com/jothepro/doxygen-awesome-css
        GIT_TAG main
    )
    # Retrieve the properties related to the content.
    FetchContent_GetProperties(doxygenawesome)
    # If not populated, make the content available.
    if(NOT doxygenawesome_POPULATED)
        message(STATUS "Retrieving `doxygen-awesome-css`...")
        # Ensures the named dependencies have been populated.
        FetchContent_MakeAvailable(doxygenawesome)
        # Hide fetchcontent variables, otherwise with ccmake it's a mess.
        mark_as_advanced(FORCE
            FETCHCONTENT_UPDATES_DISCONNECTED_DOXYGENAWESOME
            FETCHCONTENT_SOURCE_DIR_DOXYGENAWESOME
        )
    endif()

    # = CUSTOMIZATION =========================================================
    set(DOXYGEN_WARN_FORMAT "$file:$line: $text")
    set(DOXYGEN_PROJECT_NAME flexman)
    set(DOXYGEN_USE_MDFILE_AS_MAINPAGE ${PROJECT_SOURCE_DIR}/README.md)
    set(DOXYGEN_SHOW_INCLUDE_FILES NO)
    set(DOXYGEN_GENERATE_TREEVIEW YES)
    set(DOXYGEN_HTML_HEADER ${doxygenawesome_SOURCE_DIR}/doxygen-custom/header.html)
    set(DOXYGEN_HTML_EXTRA_STYLESHEET ${doxygenawesome_SOURCE_DIR}/doxygen-awesome.css)
    set(DOXYGEN_HTML_EXTRA_FILES
        ${doxygenawesome_SOURCE_DIR}/doxygen-awesome-fragment-copy-button.js
        ${doxygenawesome_SOURCE_DIR}/doxygen-awesome-paragraph-link.js
        ${doxygenawesome_SOURCE_DIR}/doxygen-awesome-darkmode-toggle.js
    )
    doxygen_add_docs(
        flexman_documentation
        ${PROJECT_SOURCE_DIR}/README.md
        ${PROJECT_SOURCE_DIR}/LICENSE.md
        ${PROJECT_SOURCE_DIR}/include/flexman/simulation/common.hpp
        ${PROJECT_SOURCE_DIR}/include/flexman/simulation/simulate.hpp
        ${PROJECT_SOURCE_DIR}/include/flexman/serialization.hpp
        ${PROJECT_SOURCE_DIR}/include/flexman/flexman.hpp
        ${PROJECT_SOURCE_DIR}/include/flexman/data_structure/manager.hpp
        ${PROJECT_SOURCE_DIR}/include/flexman/data_structure/mode_execution.hpp
        ${PROJECT_SOURCE_DIR}/include/flexman/data_structure/pareto_front.hpp
        ${PROJECT_SOURCE_DIR}/include/flexman/data_structure/solution.hpp
        ${PROJECT_SOURCE_DIR}/include/flexman/data_structure/result.hpp
        ${PROJECT_SOURCE_DIR}/include/flexman/data_structure/mode.hpp
        ${PROJECT_SOURCE_DIR}/include/flexman/logging.hpp
        ${PROJECT_SOURCE_DIR}/include/flexman/search/search.hpp
        ${PROJECT_SOURCE_DIR}/include/flexman/search/common.hpp
        ${PROJECT_SOURCE_DIR}/include/flexman/pso/optimize.hpp
        ${PROJECT_SOURCE_DIR}/include/flexman/pso/common.hpp
    )
endif()
