cmake_minimum_required(VERSION 3.24)
project(Monopoly C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -lncurses -lm")

add_executable(main.so src/main.c)
target_include_directories(main.so PRIVATE ${PROJECT_SOURCE_DIR}/include)