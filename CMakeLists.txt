cmake_minimum_required(VERSION 3.18)

project(
    Visit
    VERSION 0.1
    DESCRIPTION "Optimized Visit for std::variant"
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_MACOSX_RPATH 1)

find_package(benchmark CONFIG REQUIRED)

add_executable(visit visit.cpp)
target_link_libraries(visit PRIVATE benchmark::benchmark benchmark::benchmark_main)
