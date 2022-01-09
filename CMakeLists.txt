cmake_minimum_required(VERSION 3.18)
project(client2 C)

set(CMAKE_C_STANDARD 99)

add_executable(client2 main.c client.c client.h)

target_link_libraries(client2 pthread)