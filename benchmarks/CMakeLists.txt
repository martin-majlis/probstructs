cmake_minimum_required(VERSION 3.11...3.17)

include(FetchContent)
if(${CMAKE_VERSION} VERSION_LESS 3.14)
    include(cmake/add_FetchContent_MakeAvailable.cmake)
endif()

FetchContent_Declare(
    googlebenchmark
    GIT_REPOSITORY https://github.com/google/benchmark.git
    GIT_TAG        v1.9.1
)
set(BENCHMARK_ENABLE_TESTING     OFF CACHE BOOL "" FORCE)
set(BENCHMARK_ENABLE_INSTALL     OFF CACHE BOOL "" FORCE)
set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googlebenchmark)

set(CMAKE_CXX_STANDARD 17)

add_executable(probstructs_benchmark probstructs_benchmark.cpp)
target_link_libraries(probstructs_benchmark
    PRIVATE benchmark::benchmark benchmark::benchmark_main probstructs)
