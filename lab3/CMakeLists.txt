cmake_minimum_required(VERSION 3.10)
project(MyProject)

set(CMAKE_CXX_STANDARD 14)


add_executable(Lab3 
    main.cpp
)

include_directories(
    structure
    test
)

target_sources(Lab3 PRIVATE
    structure/fine_grained_set.cpp
    structure/node.cpp
    test/test.cpp
)

target_compile_options(Lab3 PRIVATE
    -Wall -Wextra -pedantic -Werror
)

find_package(Threads REQUIRED)
target_link_libraries(Lab3 PRIVATE Threads::Threads)
