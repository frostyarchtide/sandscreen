cmake_minimum_required(VERSION 3.31)
project(sandscreen)

add_executable(${PROJECT_NAME}
    src/main.c
)

target_link_libraries(${PROJECT_NAME} ncurses)

target_compile_options(${PROJECT_NAME} PRIVATE -O2 -Wall)
