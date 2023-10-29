cmake_minimum_required(VERSION 3.8)

project(Daemon)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-Wall -Werror")

set(PROJECT_SOURCES
        main.cpp
        Daemon.cpp Daemon.hpp
        Config.cpp Config.hpp
)

add_executable(${PROJECT_NAME}
    ${PROJECT_SOURCES})