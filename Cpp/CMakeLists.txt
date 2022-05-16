cmake_minimum_required(VERSION 3.20)
project(um_chess)

set(CMAKE_CXX_STANDARD 20)

file(GLOB_RECURSE CHESS_RULES_SRCS CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/chess_rules/*.cpp)
include_directories(${PROJECT_SOURCE_DIR}/chess_rules)
add_executable(${PROJECT_NAME} main.cpp ${THC_CHESS_SRCS})
