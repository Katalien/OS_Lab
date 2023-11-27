cmake_minimum_required(VERSION 3.16)
project(lab2)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-Wall -Werror)

if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

find_package (Threads)
find_library (LIBRT rt)

file(GLOB CONNECTIONS connections/conn_*.hpp)
file(GLOB HOST proc_queue.hpp host/*.cpp host/*.hpp)
file(GLOB CLIENT proc_queue.hpp client/*.cpp client/*.hpp)

foreach(PATH_TO_CON ${CONNECTIONS})
    get_filename_component(FILE_NAME ${PATH_TO_CON} NAME_WLE )
    string(REGEX REPLACE "conn_" "" CONN_NAME ${FILE_NAME})

    add_executable(host_${CONN_NAME} ${HOST}
            connections/connection.hpp connections/conn_${CONN_NAME}.cpp connections/conn_${CONN_NAME}.hpp)
    add_executable(client_${CONN_NAME}  ${CLIENT}
            connections/connection.hpp connections/conn_${CONN_NAME}.cpp connections/conn_${CONN_NAME}.hpp host/host.hpp)
    target_link_libraries(host_${CONN_NAME} PUBLIC pthread rt )
    target_link_libraries(client_${CONN_NAME} PUBLIC pthread rt )
endforeach()