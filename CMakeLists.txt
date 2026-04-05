cmake_minimum_required(VERSION 3.14)

project("cli")

set(CMAKE_CXX_STANDARD			23)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS		OFF)

add_library(cli STATIC
  cli/src/Console.cpp
)

target_include_directories(cli PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/cli/include")

add_executable(exec ./src/commands.cpp)
target_link_libraries(exec cli)
