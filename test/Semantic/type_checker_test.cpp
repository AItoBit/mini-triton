#include "MiniTriton/Frontend/Lexer.h"
#include "MiniTriton/Frontend/Parser.h"
#include "MiniTriton/Semantic/TypeChecker.h"
#include <iostream>
#include <cassert>

using namespace minitriton;

static int testsPassed = 0;
static int testsFailed = 0;

#define ASSERT_TRUE(cond) \
    if (!(cond)) { \
        std::cerr << "FAIL: " << #cond << " (line " << __LINE__ << ")\n"; \
        testsFailed++; return; \
    }

#define PASS(name) \
    std::cout << "  PASS: " << name << "\n"; testsPassed++;

// Helper: parse + type-check, return success
static bool typeCheck(const std::string& source, bool printErrors = false) {
    Lexer lexer(source, "<test>");
    auto tokens = lexer.tokenize();
    Parser parser(tokens, "<test>");
    auto module = parser.parseModule();
    if (parser.hasErrors()) {
        if (printErrors) {
            for (auto& e : parser.getErrors()) std::cerr << "  [parse] " << e << "\n";
        }
        return false;
    }
    TypeChecker checker;
    bool ok = checker.check(*module);
    if (!ok && printErrors) {
        for (auto& e : checker.getErrors()) std::cerr << "  [type] " << e << "\n";
    }
    return ok;
}

// -----------------------------------------------------------------------
// Tests — valid programs
// -----------------------------------------------------------------------

void test_valid_vector_add() {
    bool ok = typeCheck(
        "@kernel\n"
        "def vector_add(A: ptr<f32>, B: ptr<f32>, C: ptr<f32>, N: i32):\n"
        "    pid = program_id(0)\n"
        "    offset = pid * BLOCK + arange(0, BLOCK)\n"
        "    mask = offset < N\n"
        "    a = load(A + offset, mask)\n"
        "    b = load(B + offset, mask)\n"
        "    store(C + offset, a + b, mask)\n"
    , true);
    ASSERT_TRUE(ok);
    PASS("valid_vector_add");
}

void test_valid_simple_arithmetic() {
    bool ok = typeCheck(
        "def f(x: i32, y: i32):\n"
        "    z = x + y\n"
        "    w = z * 2\n"
    );
    ASSERT_TRUE(ok);
    PASS("valid_simple_arithmetic");
}

void test_valid_float_arithmetic() {
    bool ok = typeCheck(
        "def f(x: f32):\n"
        "    y = x + 1.0\n"
    );
    ASSERT_TRUE(ok);
    PASS("valid_float_arithmetic");
}

// -----------------------------------------------------------------------
// Tests — type errors
// -----------------------------------------------------------------------

void test_error_undefined_variable() {
    bool ok = typeCheck(
        "def f(x: i32):\n"
        "    y = z + 1\n"
    );
    ASSERT_TRUE(!ok);
    PASS("error_undefined_variable");
}

void test_error_store_wrong_arg() {
    // store with only 1 argument
    bool ok = typeCheck(
        "def f(x: i32):\n"
        "    store(x)\n"
    );
    ASSERT_TRUE(!ok);
    PASS("error_store_wrong_arg");
}

void test_error_program_id_wrong_args() {
    bool ok = typeCheck(
        "def f(x: i32):\n"
        "    pid = program_id(0, 1)\n"
    );
    ASSERT_TRUE(!ok);
    PASS("error_program_id_wrong_args");
}

void test_error_unknown_function() {
    bool ok = typeCheck(
        "def f(x: i32):\n"
        "    y = unknown_func(x)\n"
    );
    ASSERT_TRUE(!ok);
    PASS("error_unknown_function");
}

void test_error_store_non_pointer() {
    // store with non-pointer first argument
    bool ok = typeCheck(
        "def f(x: i32, y: f32):\n"
        "    store(x, y)\n"
    );
    ASSERT_TRUE(!ok);
    PASS("error_store_non_pointer");
}

int main() {
    std::cout << "=== MiniTriton Type Checker Tests ===\n\n";

    // Valid programs
    test_valid_vector_add();
    test_valid_simple_arithmetic();
    test_valid_float_arithmetic();

    // Type errors
    test_error_undefined_variable();
    test_error_store_wrong_arg();
    test_error_program_id_wrong_args();
    test_error_unknown_function();
    test_error_store_non_pointer();

    std::cout << "\n--- Results ---\n"
              << "  Passed: " << testsPassed << "\n"
              << "  Failed: " << testsFailed << "\n";

    return testsFailed > 0 ? 1 : 0;
}
