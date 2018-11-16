cmake_minimum_required(VERSION 3.12)
project(codefes)

set(CMAKE_CXX_STANDARD 17)
add_compile_options(-Wall -Wextra -march=native)
add_executable(main main.cpp)
add_executable(interactive interactive.cpp)
add_executable(greedy greedy.cpp)
