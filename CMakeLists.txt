cmake_minimum_required(VERSION 3.16)
project(lru-kvstore LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    add_compile_options(-Wno-volatile -Wno-padded -Wno-psabi)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp)
add_library(kvstore_lib ${SOURCES})
target_include_directories(kvstore_lib PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.zip
)
set(INSTALL_GTEST OFF)
FetchContent_MakeAvailable(googletest)

enable_testing()

file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS tests/*.cpp)
add_executable(kvstore_tests ${TEST_SOURCES})
target_link_libraries(kvstore_tests
        kvstore_lib
        gtest_main
)
target_include_directories(kvstore_tests PRIVATE include)

include(GoogleTest)
gtest_discover_tests(kvstore_tests)

# Benchmark executable
FetchContent_Declare(
        benchmark
        URL https://github.com/google/benchmark/archive/refs/tags/v1.9.4.zip
)
set(BENCHMARK_ENABLE_TESTING OFF)
FetchContent_MakeAvailable(benchmark)


file(GLOB_RECURSE BENCH_SOURCES CONFIGURE_DEPENDS bench/*.cpp)
add_executable(kvstore_bench ${BENCH_SOURCES})
target_link_libraries(kvstore_bench kvstore_lib benchmark)
target_include_directories(kvstore_bench PRIVATE include)
