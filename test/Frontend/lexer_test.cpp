#include "MiniTriton/Frontend/Lexer.h"
#include <iostream>
#include <cassert>
#include <vector>
#include <string>

using namespace minitriton;

// Simple test framework
static int testsPassed = 0;
static int testsFailed = 0;

#define TEST(name) \
    static void test_##name(); \
    struct TestReg_##name { TestReg_##name() { test_##name(); } } reg_##name; \
    static void test_##name()

#define ASSERT_EQ(a, b) \
    if ((a) != (b)) { \
        std::cerr << "FAIL: " << #a << " != " << #b \
                  << " (line " << __LINE__ << ")\n"; \
        testsFailed++; return; \
    }

#define PASS(name) \
    std::cout << "  PASS: " << name << "\n"; testsPassed++;

// -----------------------------------------------------------------------
// Tests
// -----------------------------------------------------------------------

TEST(empty_source) {
    Lexer lexer("");
    auto tokens = lexer.tokenize();
    ASSERT_EQ(tokens.back().kind, TokenKind::EndOfFile);
    PASS("empty_source");
}

TEST(simple_identifier) {
    Lexer lexer("hello");
    auto tokens = lexer.tokenize();
    ASSERT_EQ(tokens[0].kind, TokenKind::Identifier);
    ASSERT_EQ(tokens[0].value, "hello");
    PASS("simple_identifier");
}

TEST(keywords) {
    Lexer lexer("def kernel load store program_id arange i32 f32 ptr");
    auto tokens = lexer.tokenize();
    ASSERT_EQ(tokens[0].kind, TokenKind::KW_def);
    ASSERT_EQ(tokens[1].kind, TokenKind::KW_kernel);
    ASSERT_EQ(tokens[2].kind, TokenKind::KW_load);
    ASSERT_EQ(tokens[3].kind, TokenKind::KW_store);
    ASSERT_EQ(tokens[4].kind, TokenKind::KW_program_id);
    ASSERT_EQ(tokens[5].kind, TokenKind::KW_arange);
    ASSERT_EQ(tokens[6].kind, TokenKind::KW_i32);
    ASSERT_EQ(tokens[7].kind, TokenKind::KW_f32);
    ASSERT_EQ(tokens[8].kind, TokenKind::KW_ptr);
    PASS("keywords");
}

TEST(number_literals) {
    Lexer lexer("42 3.14 0 256");
    auto tokens = lexer.tokenize();
    ASSERT_EQ(tokens[0].kind, TokenKind::Integer);
    ASSERT_EQ(tokens[0].value, "42");
    ASSERT_EQ(tokens[1].kind, TokenKind::Float);
    ASSERT_EQ(tokens[1].value, "3.14");
    ASSERT_EQ(tokens[2].kind, TokenKind::Integer);
    ASSERT_EQ(tokens[3].kind, TokenKind::Integer);
    ASSERT_EQ(tokens[3].value, "256");
    PASS("number_literals");
}

TEST(operators) {
    Lexer lexer("+ - * / < > <= >= == !=");
    auto tokens = lexer.tokenize();
    ASSERT_EQ(tokens[0].kind, TokenKind::Plus);
    ASSERT_EQ(tokens[1].kind, TokenKind::Minus);
    ASSERT_EQ(tokens[2].kind, TokenKind::Star);
    ASSERT_EQ(tokens[3].kind, TokenKind::Slash);
    ASSERT_EQ(tokens[4].kind, TokenKind::Less);
    ASSERT_EQ(tokens[5].kind, TokenKind::Greater);
    ASSERT_EQ(tokens[6].kind, TokenKind::LessEqual);
    ASSERT_EQ(tokens[7].kind, TokenKind::GreaterEqual);
    ASSERT_EQ(tokens[8].kind, TokenKind::EqualEqual);
    ASSERT_EQ(tokens[9].kind, TokenKind::NotEqual);
    PASS("operators");
}

TEST(punctuation) {
    Lexer lexer("@ : , = ( )");
    auto tokens = lexer.tokenize();
    ASSERT_EQ(tokens[0].kind, TokenKind::At);
    ASSERT_EQ(tokens[1].kind, TokenKind::Colon);
    ASSERT_EQ(tokens[2].kind, TokenKind::Comma);
    ASSERT_EQ(tokens[3].kind, TokenKind::Equal);
    ASSERT_EQ(tokens[4].kind, TokenKind::LParen);
    ASSERT_EQ(tokens[5].kind, TokenKind::RParen);
    PASS("punctuation");
}

TEST(decorator) {
    Lexer lexer("@kernel\ndef foo():");
    auto tokens = lexer.tokenize();
    ASSERT_EQ(tokens[0].kind, TokenKind::At);
    ASSERT_EQ(tokens[1].kind, TokenKind::KW_kernel);
    PASS("decorator");
}

TEST(indentation) {
    Lexer lexer("def f():\n    x = 1\n    y = 2\n");
    auto tokens = lexer.tokenize();

    // Should contain: def, f, (, ), :, Newline, Indent, x, =, 1, Newline, y, =, 2, Newline, Dedent, EOF
    bool hasIndent = false, hasDedent = false;
    for (auto& t : tokens) {
        if (t.kind == TokenKind::Indent) hasIndent = true;
        if (t.kind == TokenKind::Dedent) hasDedent = true;
    }
    ASSERT_EQ(hasIndent, true);
    ASSERT_EQ(hasDedent, true);
    PASS("indentation");
}

TEST(comments) {
    Lexer lexer("x = 1  # this is a comment\ny = 2");
    auto tokens = lexer.tokenize();
    // Comment should be skipped
    bool hasComment = false;
    for (auto& t : tokens) {
        if (t.value.find("comment") != std::string::npos) hasComment = true;
    }
    ASSERT_EQ(hasComment, false);
    PASS("comments");
}

TEST(type_annotation) {
    Lexer lexer("ptr<f32>");
    auto tokens = lexer.tokenize();
    ASSERT_EQ(tokens[0].kind, TokenKind::KW_ptr);
    ASSERT_EQ(tokens[1].kind, TokenKind::Less);
    ASSERT_EQ(tokens[2].kind, TokenKind::KW_f32);
    ASSERT_EQ(tokens[3].kind, TokenKind::Greater);
    PASS("type_annotation");
}

int main() {
    std::cout << "=== MiniTriton Lexer Tests ===\n\n";

    // Tests are auto-registered and already ran

    std::cout << "\n--- Results ---\n"
              << "  Passed: " << testsPassed << "\n"
              << "  Failed: " << testsFailed << "\n";

    return testsFailed > 0 ? 1 : 0;
}
