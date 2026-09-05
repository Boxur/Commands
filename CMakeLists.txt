cmake_minimum_required(VERSION 3.14)

project("cli")

set(CMAKE_EXPORT_COMPILE_COMMANDS YES)
set(CMAKE_CXX_STANDARD			23)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS		OFF)

add_library(cli STATIC
  src/console.cpp
)

target_include_directories(cli PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

add_executable(exec src/commands.cpp)
target_link_libraries(exec cli)
