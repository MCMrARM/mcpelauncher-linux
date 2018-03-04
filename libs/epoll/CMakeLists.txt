cmake_minimum_required(VERSION 2.8)
project(epoll C)

set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} -m32")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++11")
set(CMAKE_LIBRARY_ARCHITECTURE "i386-linux-gnu")

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_EXTENSIONS ON)

add_subdirectory(src)
