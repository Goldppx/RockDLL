cmake_minimum_required(VERSION 3.15)
project(RockDLL VERSION 1.0.0 LANGUAGES C)

add_definitions(-DUNICODE -D_UNICODE)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-DDEBUG)
endif()

add_executable(RockDLL src/main.cpp)
