cmake_minimum_required(VERSION 3.21)
project(exp_cpp)

set(CMAKE_CXX_STANDARD 11)

aux_source_directory(./src SRC)

add_executable(${PROJECT_NAME} ${SRC} ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)