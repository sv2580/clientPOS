cmake_minimum_required(VERSION 3.18)
project(untitled1 C)

set(CMAKE_C_STANDARD 99)

add_executable(clientPOS main.c)

target_link_libraries(clientPOS pthread)