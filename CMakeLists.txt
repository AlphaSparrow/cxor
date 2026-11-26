cmake_minimum_required(VERSION 3.20)

project(CXOR VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(cxor_core
    src/Packet.cpp
    src/Encoder.cpp
)

target_include_directories(cxor_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if(MSVC)
    target_compile_options(cxor_core PRIVATE /W4 /permissive-)
else()
    target_compile_options(cxor_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(CXOR
    src/main.cpp
)
target_link_libraries(CXOR PRIVATE cxor_core)

enable_testing()
add_executable(cxor_tests
    tests/tests.cpp
)
target_link_libraries(cxor_tests PRIVATE cxor_core)
add_test(NAME cxor_tests COMMAND cxor_tests)
