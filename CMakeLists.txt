cmake_minimum_required(VERSION 3.10)
project(CVM++ LANGUAGES CXX)

# Use C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Collect all source files
add_executable(cvm++
    main.cpp
    lexer.cpp
    parser.cpp
    compiler.cpp
    vm.cpp
    debug.cpp
)

# Include current directory for headers
target_include_directories(cvm++ PRIVATE ${CMAKE_SOURCE_DIR})