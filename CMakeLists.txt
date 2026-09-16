cmake_minimum_required(VERSION 3.20)
project(MiniTriton VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# --- Options ---
option(MT_ENABLE_TESTS "Build MiniTriton tests" ON)

# --- CMake modules ---
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")

# Try to find MLIR (optional for frontend, required for backend)
find_package(MLIR CONFIG)
if(MLIR_FOUND)
    message(STATUS "Found MLIR: ${MLIR_DIR}")
    include(${CMAKE_SOURCE_DIR}/cmake/modules/FindMLIR.cmake)
    set(MT_HAS_MLIR ON)
else()
    message(WARNING "MLIR not found. Building frontend only.")
    set(MT_HAS_MLIR OFF)
endif()

# --- Include paths ---
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_BINARY_DIR}/include)

# --- Frontend library ---
add_library(MTFrontend
    lib/Frontend/Lexer.cpp
    lib/Frontend/Parser.cpp
    lib/AST/AST.cpp
    lib/AST/ASTprinter.cpp
    lib/Semantic/TypeChecker.cpp
)
target_include_directories(MTFrontend PUBLIC ${CMAKE_SOURCE_DIR}/include)

# --- MLIR Backend ---
if(MT_HAS_MLIR)
    # TableGen for Dialect
    set(LLVM_TARGET_DEFINITIONS include/MiniTriton/Dialect/MiniTritonOps.td)
    mlir_tablegen(MiniTritonOps.h.inc -gen-op-decls)
    mlir_tablegen(MiniTritonOps.cpp.inc -gen-op-defs)
    add_public_tablegen_target(MiniTritonOpsIncGen)

    set(LLVM_TARGET_DEFINITIONS include/MiniTriton/Dialect/MiniTritonDialect.td)
    mlir_tablegen(MiniTritonDialect.h.inc -gen-dialect-decls)
    mlir_tablegen(MiniTritonDialect.cpp.inc -gen-dialect-defs)
    mlir_tablegen(MiniTritonTypes.h.inc -gen-typedef-decls -typedefs-dialect=mt)
    mlir_tablegen(MiniTritonTypes.cpp.inc -gen-typedef-defs -typedefs-dialect=mt)
    add_public_tablegen_target(MiniTritonDialectIncGen)

    # Dialect Library
    add_library(MTDialect
        lib/Dialect/MiniTritonDialect.cpp
    )
    add_dependencies(MTDialect MiniTritonOpsIncGen MiniTritonDialectIncGen)
    target_link_libraries(MTDialect ${MLIR_LIBS})

    # CodeGen Library
    add_library(MTCodeGen
        lib/CodeGen/MLIRGen.cpp
    )
    target_link_libraries(MTCodeGen MTFrontend MTDialect ${MLIR_LIBS})
endif()

# --- Main driver ---
add_executable(minitriton tools/minitriton/main.cpp)
if(MT_HAS_MLIR)
    target_link_libraries(minitriton PRIVATE MTFrontend MTCodeGen ${MLIR_LIBS})
    target_compile_definitions(minitriton PRIVATE MT_HAS_MLIR)
else()
    target_link_libraries(minitriton PRIVATE MTFrontend)
endif()

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

    add_executable(benchmark tools/benchmark/benchmark.cpp)
    add_test(NAME Benchmark COMMAND benchmark)
endif()
