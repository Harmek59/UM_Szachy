cmake_minimum_required(VERSION 3.20)
project(ChessEngine)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/engineDll)

file(GLOB_RECURSE THC_CHESS_SRCS CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/chess_rules/*.cpp)
include_directories(${PROJECT_SOURCE_DIR}/chess_rules)
add_compile_options("-fdeclspec")

if (DEFINED BUILD_AS_TESTS_EXE)
    add_executable(${PROJECT_NAME} main.cpp ${THC_CHESS_SRCS})
else ()
    add_library(${PROJECT_NAME} MODULE main.cpp ${THC_CHESS_SRCS})
endif ()
