cmake_minimum_required(VERSION 3.16)
project(moschus LANGUAGES CXX)

# NEED 20 for format
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

enable_testing()

find_package(argparse REQUIRED)

# remove after generating lexer
find_package(FLEX REQUIRED)
find_package(GTest REQUIRED)

# Flex
FLEX_TARGET(
    Lexer
    assets/lexer.l
    ${CMAKE_CURRENT_BINARY_DIR}/../src/parser/frontend/musk_lexer.cpp
)

add_executable(${PROJECT_NAME}
    src/moschus.cpp
    src/musk_template.cpp
    ${FLEX_Lexer_OUTPUTS}
)

add_executable(tests
    tests/lexer_test.cpp
)

target_link_libraries(${PROJECT_NAME} 
    PRIVATE argparse::argparse
)

target_link_libraries(tests
    PRIVATE GTest::gtest_main
)

target_include_directories(${PROJECT_NAME} PRIVATE
    src
)

include(GoogleTest)
gtest_discover_tests(tests)