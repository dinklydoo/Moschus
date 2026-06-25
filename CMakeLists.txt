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
    DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/../src/parser/frontend/musk_lexer.hpp
)

add_library(moschus_lib
    src/errors/except_handler.cpp
    src/errors/format_errors.cpp

    src/parser/parse_gen.cpp
    src/musk_template.cpp
    src/parser/frontend/musk_parser.cpp
    src/parser/preprocess_member.cpp
    src/parser/preprocess_impl.cpp
    src/parser/table_gen.cpp
    src/parser/code_gen.cpp
    ${FLEX_Lexer_OUTPUTS}
)

add_executable(${PROJECT_NAME}
    src/moschus.cpp
)

add_executable(tests
    tests/musklex_test.cpp
    tests/muskparse_test.cpp
    tests/first_follow_test.cpp
    tests/table_test.cpp
    tests/codegen_test.cpp
)

target_link_libraries(${PROJECT_NAME} 
    PRIVATE argparse::argparse
    PRIVATE moschus_lib
)

target_link_libraries(tests
    PRIVATE GTest::gtest_main
    PRIVATE moschus_lib
)

target_include_directories(${PROJECT_NAME} PRIVATE
    src
)

include(GoogleTest)
gtest_discover_tests(tests)
