cmake_minimum_required(VERSION 3.16)
project(SoCPerfSim VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(socperfsim_lib
    src/cache.cpp
    src/trace.cpp
    src/simulator.cpp
)

target_include_directories(socperfsim_lib PUBLIC include)

if (MSVC)
    target_compile_options(socperfsim_lib PRIVATE /W4)
else()
    target_compile_options(socperfsim_lib PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(socperfsim src/main.cpp)
target_link_libraries(socperfsim PRIVATE socperfsim_lib)

enable_testing()
add_executable(test_cache tests/test_cache.cpp)
target_link_libraries(test_cache PRIVATE socperfsim_lib)
add_test(NAME cache_tests COMMAND test_cache)
