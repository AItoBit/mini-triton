cmake_minimum_required(VERSION 3.20)
project(MiniTriton VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# --- Options ---
option(MT_ENABLE_TESTS "Build MiniTriton tests" ON)

# --- Include paths ---
include_directories(${CMAKE_SOURCE_DIR}/include)

# --- Frontend library ---
add_library(MTFrontend
    lib/Frontend/Lexer.cpp
    lib/Frontend/Parser.cpp
    lib/AST/AST.cpp
    lib/AST/ASTprinter.cpp
    lib/Semantic/TypeChecker.cpp
)
target_include_directories(MTFrontend PUBLIC ${CMAKE_SOURCE_DIR}/include)

# --- Main driver ---
add_executable(minitriton tools/minitriton/main.cpp)
target_link_libraries(minitriton PRIVATE MTFrontend)

# --- Tests ---
if(MT_ENABLE_TESTS)
    enable_testing()

    add_executable(lexer_test test/Frontend/lexer_test.cpp)
    target_link_libraries(lexer_test PRIVATE MTFrontend)
    add_test(NAME LexerTest COMMAND lexer_test)

    add_executable(parser_test test/Frontend/parser_test.cpp)
    target_link_libraries(parser_test PRIVATE MTFrontend)
    add_test(NAME ParserTest COMMAND parser_test)

    add_executable(type_checker_test test/Semantic/type_checker_test.cpp)
    target_link_libraries(type_checker_test PRIVATE MTFrontend)
    add_test(NAME TypeCheckerTest COMMAND type_checker_test)
endif()
