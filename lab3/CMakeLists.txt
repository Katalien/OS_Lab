# cmake_minimum_required(VERSION 3.10)
# project(MyProject)

# set(CMAKE_CXX_STANDARD 14)

# file(GLOB SRC "src/*.cpp")

# add_compile_options(-Wall -Werror)

# add_executable(MyProjectExecutable ${SRC})

# target_link_libraries(MyProjectExecutable rt pthread)

cmake_minimum_required(VERSION 3.10)
project(MyProject)

set(CMAKE_CXX_STANDARD 14)

# Добавляем исполняемый файл
add_executable(MyProjectExecutable 
    main.cpp
)

# Добавляем пути к инклюдам для ваших заголовочных файлов и исходных файлов
include_directories(
    structure
    test
)

# Добавляем исходные файлы из структуры и тестов
target_sources(MyProjectExecutable PRIVATE
    structure/fine_grained_set.cpp
    structure/node.cpp
    test/test.cpp
)

# Включаем компиляцию с предупреждениями
target_compile_options(MyProjectExecutable PRIVATE
    -Wall -Wextra -pedantic -Werror
)

# Линкуем с библиотекой pthread
find_package(Threads REQUIRED)
target_link_libraries(MyProjectExecutable PRIVATE Threads::Threads)
