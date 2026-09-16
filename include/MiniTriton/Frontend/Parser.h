#pragma once

#include "MiniTriton/Frontend/Token.h"
#include "MiniTriton/AST/AST.h"
#include <vector>
#include <string>
#include <memory>
#include <functional>

namespace minitriton {

class Parser {
public:
    explicit Parser(const std::vector<Token>& tokens, const std::string& filename = "<input>");

    /// Parse the full module (top-level entry point).
    std::unique_ptr<Module> parseModule();

    /// Check if there were any errors during parsing.
    bool hasErrors() const { return !errors_.empty(); }
    const std::vector<std::string>& getErrors() const { return errors_; }

private:
    std::vector<Token> tokens_;
    std::string filename_;
    size_t pos_;
    std::vector<std::string> errors_;

    // Token navigation
    const Token& peek() const;
    const Token& peekNext() const;
    const Token& previous() const;
    const Token& advance();
    bool isAtEnd() const;
    bool check(TokenKind kind) const;
    bool match(TokenKind kind);
    Token expect(TokenKind kind, const std::string& message);
    void skipNewlines();

    // Error handling
    void error(const std::string& message);
    void synchronize();

    // Grammar rules
    std::unique_ptr<FunctionDef> parseFunction();
    std::vector<Parameter> parseParameters();
    std::shared_ptr<Type> parseType();
    std::vector<StmtPtr> parseBlock();
    StmtPtr parseStatement();
    StmtPtr parseAssignOrExprStmt();
    StmtPtr parseReturnStmt();

    // Expression parsing (operator precedence)
    ExprPtr parseExpression();
    ExprPtr parseComparison();
    ExprPtr parseAddition();
    ExprPtr parseMultiplication();
    ExprPtr parseUnary();
    ExprPtr parseCall();
    ExprPtr parsePrimary();
    std::vector<ExprPtr> parseArgList();
};

} // namespace minitriton
