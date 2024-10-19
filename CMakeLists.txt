cmake_minimum_required(VERSION 3.29)
project(dazo C)

set(CMAKE_C_STANDARD 23)

add_executable(dazo
        src/main.c
        src/log.c
        src/log.h)
