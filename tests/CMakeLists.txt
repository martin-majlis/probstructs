cmake_minimum_required(VERSION 3.11...3.17)

include(FetchContent)
if(${CMAKE_VERSION} VERSION_LESS 3.14)
    include(cmake/add_FetchContent_MakeAvailable.cmake)
endif()

find_package(Threads REQUIRED)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.14.0
    GIT_SHALLOW    ON
)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

set(CMAKE_CXX_STANDARD 17)

add_executable(probstructs_test gtest_main.cpp probstructs_unittest.cpp)
target_link_libraries(probstructs_test gtest probstructs)

enable_testing()
add_test(NAME probstructs_test COMMAND probstructs_test)
