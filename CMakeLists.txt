cmake_minimum_required(VERSION 3.18)
project(client C)

set(CMAKE_C_STANDARD 99)

add_executable(client main.c client.c client.h)

target_link_libraries(client pthread)