cmake_minimum_required(VERSION 3.16)
project(moschus LANGUAGES CXX)

# NEED 20 for format
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(argparse REQUIRED)

add_executable(${PROJECT_NAME}
    src/moschus.cpp
    src/musk_template.cpp
)

target_link_libraries(${PROJECT_NAME} 
    PRIVATE argparse::argparse
)

target_include_directories(${PROJECT_NAME} PRIVATE
    src
)