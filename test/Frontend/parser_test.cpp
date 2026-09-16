#include "MiniTriton/Frontend/Lexer.h"
#include "MiniTriton/Frontend/Parser.h"
#include "MiniTriton/AST/ASTPrinter.h"
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

#define ASSERT_EQ(a, b) \
    if ((a) != (b)) { \
        std::cerr << "FAIL: " << #a << " != " << #b \
                  << " (line " << __LINE__ << ")\n"; \
        testsFailed++; return; \
    }

#define PASS(name) \
    std::cout << "  PASS: " << name << "\n"; testsPassed++;

// Helper: parse source and return module
static std::unique_ptr<Module> parse(const std::string& source, bool expectErrors = false) {
    Lexer lexer(source, "<test>");
    auto tokens = lexer.tokenize();
    Parser parser(tokens, "<test>");
    auto module = parser.parseModule();
    if (!expectErrors) {
        for (const auto& err : parser.getErrors()) {
            std::cerr << "  [parse error] " << err << "\n";
        }
        if (parser.hasErrors()) {
            testsFailed++;
            return nullptr;
        }
    }
    return module;
}

// -----------------------------------------------------------------------
// Tests
// -----------------------------------------------------------------------

void test_simple_function() {
    auto mod = parse(
        "def foo(x: i32):\n"
        "    y = x + 1\n"
    );
    ASSERT_TRUE(mod != nullptr);
    ASSERT_EQ(mod->functions.size(), (size_t)1);
    ASSERT_EQ(mod->functions[0]->name, "foo");
    ASSERT_EQ(mod->functions[0]->isKernel, false);
    ASSERT_EQ(mod->functions[0]->params.size(), (size_t)1);
    ASSERT_EQ(mod->functions[0]->params[0].name, "x");
    PASS("simple_function");
}

void test_kernel_decorator() {
    auto mod = parse(
        "@kernel\n"
        "def vec_add(A: ptr<f32>, B: ptr<f32>):\n"
        "    x = 1\n"
    );
    ASSERT_TRUE(mod != nullptr);
    ASSERT_EQ(mod->functions[0]->isKernel, true);
    ASSERT_EQ(mod->functions[0]->name, "vec_add");
    ASSERT_EQ(mod->functions[0]->params.size(), (size_t)2);
    PASS("kernel_decorator");
}

void test_pointer_type() {
    auto mod = parse(
        "def f(A: ptr<f32>):\n"
        "    x = 1\n"
    );
    ASSERT_TRUE(mod != nullptr);
    ASSERT_TRUE(mod->functions[0]->params[0].type->isPointer());
    ASSERT_EQ(mod->functions[0]->params[0].type->elementType->kind, TypeKind::F32);
    PASS("pointer_type");
}

void test_binary_expression() {
    auto mod = parse(
        "def f(x: i32):\n"
        "    y = x + 1\n"
        "    z = y * 2\n"
    );
    ASSERT_TRUE(mod != nullptr);
    ASSERT_EQ(mod->functions[0]->body.size(), (size_t)2);
    PASS("binary_expression");
}

void test_function_call() {
    auto mod = parse(
        "def f(A: ptr<f32>):\n"
        "    pid = program_id(0)\n"
        "    a = load(A, pid)\n"
    );
    ASSERT_TRUE(mod != nullptr);
    ASSERT_EQ(mod->functions[0]->body.size(), (size_t)2);
    PASS("function_call");
}

void test_comparison() {
    auto mod = parse(
        "def f(x: i32, N: i32):\n"
        "    mask = x < N\n"
    );
    ASSERT_TRUE(mod != nullptr);
    PASS("comparison");
}

void test_ast_printer() {
    auto mod = parse(
        "@kernel\n"
        "def add(A: ptr<f32>, B: ptr<f32>):\n"
        "    x = load(A)\n"
        "    y = load(B)\n"
        "    z = x + y\n"
    );
    ASSERT_TRUE(mod != nullptr);

    ASTPrinter printer;
    std::string output = printer.print(*mod);
    ASSERT_TRUE(output.find("@kernel") != std::string::npos);
    ASSERT_TRUE(output.find("add") != std::string::npos);
    ASSERT_TRUE(output.find("load") != std::string::npos);
    PASS("ast_printer");
}

void test_store_statement() {
    auto mod = parse(
        "def f(C: ptr<f32>):\n"
        "    store(C, 42)\n"
    );
    ASSERT_TRUE(mod != nullptr);
    ASSERT_EQ(mod->functions[0]->body.size(), (size_t)1);
    PASS("store_statement");
}

int main() {
    std::cout << "=== MiniTriton Parser Tests ===\n\n";

    test_simple_function();
    test_kernel_decorator();
    test_pointer_type();
    test_binary_expression();
    test_function_call();
    test_comparison();
    test_ast_printer();
    test_store_statement();

    std::cout << "\n--- Results ---\n"
              << "  Passed: " << testsPassed << "\n"
              << "  Failed: " << testsFailed << "\n";

    return testsFailed > 0 ? 1 : 0;
}
