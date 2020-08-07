# inspired by https://gitlab.com/CLIUtils/modern-cmake/-/tree/master/examples/extended-project

# Works with 3.11 and tested through 3.17
cmake_minimum_required(VERSION 3.11...3.17)

# Project name and a few useful settings. Other commands can pick up the results
project("ProbStructs"
        VERSION 0.4.0
        DESCRIPTION "Library with probabilistic data structures"
        LANGUAGES CXX)

# FetchContent added in CMake 3.11, downloads during the configure step
include(FetchContent)
# FetchContent_MakeAvailable was not added until CMake 3.14; use our shim
if(${CMAKE_VERSION} VERSION_LESS 3.14)
    include(cmake/add_FetchContent_MakeAvailable.cmake)
endif()


# Accumulator library
# This is header only, so could be replaced with git submodules or FetchContent
# find_package(Boost REQUIRED)
# Adds Boost::boost

# Formatting library
FetchContent_Declare(
        fmtlib
        GIT_REPOSITORY https://github.com/fmtlib/fmt.git
        GIT_TAG        5.3.0
)
FetchContent_MakeAvailable(fmtlib)
# Adds fmt::fmt

# The compiled library code is here
add_subdirectory(probstructs)

# Only do these if this is the main project, and not if it is included through add_subdirectory
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)

    # Optionally set things like CMAKE_CXX_STANDARD, CMAKE_POSITION_INDEPENDENT_CODE here

    # Let's ensure -std=c++xx instead of -std=g++xx
    set(CMAKE_CXX_EXTENSIONS OFF)

    # Let's nicely support folders in IDEs
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)

    # Testing only available if this is the main app
    # Note this needs to be done in the main CMakeLists
    # since it calls enable_testing, which must be in the
    # main CMakeLists.
    include(CTest)

    # Docs only available if this is the main app
    find_package(Doxygen)
    if(Doxygen_FOUND)

        # Add the cmake folder so the FindSphinx module is found
        set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

        add_subdirectory(docs)

        get_target_property(PROBSTRUCTS_PUBLIC_HEADER_DIR probstructs INTERFACE_INCLUDE_DIRECTORIES)
        file(GLOB_RECURSE PROBSTRUCTS_PUBLIC_HEADERS ${PROBSTRUCTS_PUBLIC_HEADER_DIR}/*.h)

        #This will be the main output of our command
        set(DOXYGEN_INDEX_FILE ${CMAKE_CURRENT_SOURCE_DIR}/html/index.html)

        add_custom_command(OUTPUT ${DOXYGEN_INDEX_FILE}
                        DEPENDS ${PROBSTRUCTS_PUBLIC_HEADERS}
                        COMMAND ${DOXYGEN_EXECUTABLE} Doxyfile
                        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                        MAIN_DEPENDENCY Doxyfile
                        COMMENT "Generating docs")

        # add_custom_target(Doxygen ALL DEPENDS ${DOXYGEN_INDEX_FILE})
    else()
        message(STATUS "Doxygen not found, not building docs")
    endif()
endif()

# Testing only available if this is the main app
# Emergency override MODERN_CMAKE_BUILD_TESTING provided as well
if((CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME OR MODERN_CMAKE_BUILD_TESTING) AND BUILD_TESTING)
    add_subdirectory(tests)
endif()
