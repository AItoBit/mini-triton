#include "MiniTriton/Frontend/Parser.h"
#include <sstream>
#include <iostream>

namespace minitriton {

// -----------------------------------------------------------------------
// Construction
// -----------------------------------------------------------------------

Parser::Parser(const std::vector<Token>& tokens, const std::string& filename)
    : tokens_(tokens), filename_(filename), pos_(0) {}

// -----------------------------------------------------------------------
// Token navigation
// -----------------------------------------------------------------------

const Token& Parser::peek() const {
    if (pos_ < tokens_.size()) return tokens_[pos_];
    static Token eof(TokenKind::EndOfFile, "", 0, 0);
    return eof;
}

const Token& Parser::peekNext() const {
    if (pos_ + 1 < tokens_.size()) return tokens_[pos_ + 1];
    static Token eof(TokenKind::EndOfFile, "", 0, 0);
    return eof;
}

const Token& Parser::previous() const {
    return tokens_[pos_ - 1];
}

const Token& Parser::advance() {
    if (!isAtEnd()) pos_++;
    return previous();
}

bool Parser::isAtEnd() const {
    return peek().kind == TokenKind::EndOfFile;
}

bool Parser::check(TokenKind kind) const {
    return peek().kind == kind;
}

bool Parser::match(TokenKind kind) {
    if (check(kind)) {
        advance();
        return true;
    }
    return false;
}

Token Parser::expect(TokenKind kind, const std::string& message) {
    if (check(kind)) {
        return advance();
    }
    error(message + " (got '" + peek().value + "' [" +
          tokenKindToString(peek().kind) + "])");
    return Token(TokenKind::Error, "", peek().line, peek().col);
}

void Parser::skipNewlines() {
    while (check(TokenKind::Newline)) {
        advance();
    }
}

// -----------------------------------------------------------------------
// Error handling
// -----------------------------------------------------------------------

void Parser::error(const std::string& message) {
    std::ostringstream oss;
    oss << filename_ << ":" << peek().line << ":" << peek().col
        << ": error: " << message;
    errors_.push_back(oss.str());
}

void Parser::synchronize() {
    // Skip tokens until we find a reasonable recovery point
    while (!isAtEnd()) {
        if (peek().kind == TokenKind::Newline ||
            peek().kind == TokenKind::Dedent ||
            peek().kind == TokenKind::KW_def) {
            return;
        }
        advance();
    }
}

// -----------------------------------------------------------------------
// Top-level: Module
// -----------------------------------------------------------------------

std::unique_ptr<Module> Parser::parseModule() {
    auto module = std::make_unique<Module>();
    module->line = 1;
    module->col = 1;

    skipNewlines();

    while (!isAtEnd()) {
        try {
            auto fn = parseFunction();
            if (fn) {
                module->functions.push_back(std::move(fn));
            }
        } catch (...) {
            synchronize();
        }
        skipNewlines();
    }

    return module;
}

// -----------------------------------------------------------------------
// Function definition
// -----------------------------------------------------------------------

std::unique_ptr<FunctionDef> Parser::parseFunction() {
    bool isKernel = false;

    // Check for @kernel decorator
    if (match(TokenKind::At)) {
        Token decoratorName = expect(TokenKind::KW_kernel, "Expected 'kernel' after '@'");
        isKernel = true;
        skipNewlines();
    }

    expect(TokenKind::KW_def, "Expected 'def'");
    Token name = expect(TokenKind::Identifier, "Expected function name");
    expect(TokenKind::LParen, "Expected '(' after function name");

    auto params = parseParameters();

    expect(TokenKind::RParen, "Expected ')' after parameters");
    expect(TokenKind::Colon, "Expected ':' after ')'");

    skipNewlines();

    auto body = parseBlock();

    auto fn = std::make_unique<FunctionDef>(
        name.value, std::move(params), std::move(body), isKernel);
    fn->line = name.line;
    fn->col = name.col;
    return fn;
}

// -----------------------------------------------------------------------
// Parameters
// -----------------------------------------------------------------------

std::vector<Parameter> Parser::parseParameters() {
    std::vector<Parameter> params;

    if (check(TokenKind::RParen)) return params;

    do {
        Token paramName = expect(TokenKind::Identifier, "Expected parameter name");
        expect(TokenKind::Colon, "Expected ':' after parameter name");
        auto paramType = parseType();
        Parameter p(paramName.value, std::move(paramType));
        p.line = paramName.line;
        p.col = paramName.col;
        params.push_back(std::move(p));
    } while (match(TokenKind::Comma));

    return params;
}

// -----------------------------------------------------------------------
// Type parsing
// -----------------------------------------------------------------------

std::shared_ptr<Type> Parser::parseType() {
    if (match(TokenKind::KW_i32)) {
        return Type::makeI32();
    }
    if (match(TokenKind::KW_f32)) {
        return Type::makeF32();
    }
    if (match(TokenKind::KW_ptr)) {
        expect(TokenKind::Less, "Expected '<' after 'ptr'");
        auto elem = parseType();
        expect(TokenKind::Greater, "Expected '>' after pointer element type");
        return Type::makePointer(std::move(elem));
    }

    error("Expected type (i32, f32, ptr<T>)");
    return std::make_shared<Type>(TypeKind::Unknown);
}

// -----------------------------------------------------------------------
// Block (indentation-based)
// -----------------------------------------------------------------------

std::vector<StmtPtr> Parser::parseBlock() {
    std::vector<StmtPtr> stmts;

    expect(TokenKind::Indent, "Expected indented block");

    while (!check(TokenKind::Dedent) && !isAtEnd()) {
        skipNewlines();
        if (check(TokenKind::Dedent) || isAtEnd()) break;

        auto stmt = parseStatement();
        if (stmt) {
            stmts.push_back(std::move(stmt));
        }
        skipNewlines();
    }

    if (check(TokenKind::Dedent)) {
        advance();
    }

    return stmts;
}

// -----------------------------------------------------------------------
// Statements
// -----------------------------------------------------------------------

StmtPtr Parser::parseStatement() {
    if (check(TokenKind::KW_return)) {
        return parseReturnStmt();
    }
    return parseAssignOrExprStmt();
}

StmtPtr Parser::parseAssignOrExprStmt() {
    // Look ahead: if it's `IDENT = expr`, it's an assignment
    if (check(TokenKind::Identifier) && peekNext().kind == TokenKind::Equal) {
        Token name = advance();  // consume identifier
        advance();               // consume '='
        auto value = parseExpression();
        auto stmt = std::make_unique<AssignStmt>(name.value, std::move(value));
        stmt->line = name.line;
        stmt->col = name.col;
        return stmt;
    }

    // Otherwise it's an expression statement (e.g., store(...))
    auto expr = parseExpression();
    auto stmt = std::make_unique<ExprStmt>(std::move(expr));
    return stmt;
}

StmtPtr Parser::parseReturnStmt() {
    Token ret = advance();  // consume 'return'

    ExprPtr value = nullptr;
    if (!check(TokenKind::Newline) && !check(TokenKind::Dedent) && !isAtEnd()) {
        value = parseExpression();
    }

    auto stmt = std::make_unique<ReturnStmt>(std::move(value));
    stmt->line = ret.line;
    stmt->col = ret.col;
    return stmt;
}

// -----------------------------------------------------------------------
// Expressions — operator precedence climbing
// -----------------------------------------------------------------------

ExprPtr Parser::parseExpression() {
    return parseComparison();
}

ExprPtr Parser::parseComparison() {
    auto left = parseAddition();

    while (check(TokenKind::Less) || check(TokenKind::Greater) ||
           check(TokenKind::LessEqual) || check(TokenKind::GreaterEqual) ||
           check(TokenKind::EqualEqual) || check(TokenKind::NotEqual)) {
        Token opTok = advance();
        auto right = parseAddition();

        BinaryExpr::Op op;
        switch (opTok.kind) {
            case TokenKind::Less:         op = BinaryExpr::Less; break;
            case TokenKind::Greater:      op = BinaryExpr::Greater; break;
            case TokenKind::LessEqual:    op = BinaryExpr::LessEq; break;
            case TokenKind::GreaterEqual: op = BinaryExpr::GreaterEq; break;
            case TokenKind::EqualEqual:   op = BinaryExpr::Equal; break;
            case TokenKind::NotEqual:     op = BinaryExpr::NotEqual; break;
            default: op = BinaryExpr::Less; break;
        }

        auto bin = std::make_unique<BinaryExpr>(op, std::move(left), std::move(right));
        bin->line = opTok.line;
        bin->col = opTok.col;
        left = std::move(bin);
    }

    return left;
}

ExprPtr Parser::parseAddition() {
    auto left = parseMultiplication();

    while (check(TokenKind::Plus) || check(TokenKind::Minus)) {
        Token opTok = advance();
        auto right = parseMultiplication();

        BinaryExpr::Op op = (opTok.kind == TokenKind::Plus)
                            ? BinaryExpr::Add : BinaryExpr::Sub;
        auto bin = std::make_unique<BinaryExpr>(op, std::move(left), std::move(right));
        bin->line = opTok.line;
        bin->col = opTok.col;
        left = std::move(bin);
    }

    return left;
}

ExprPtr Parser::parseMultiplication() {
    auto left = parseUnary();

    while (check(TokenKind::Star) || check(TokenKind::Slash)) {
        Token opTok = advance();
        auto right = parseUnary();

        BinaryExpr::Op op = (opTok.kind == TokenKind::Star)
                            ? BinaryExpr::Mul : BinaryExpr::Div;
        auto bin = std::make_unique<BinaryExpr>(op, std::move(left), std::move(right));
        bin->line = opTok.line;
        bin->col = opTok.col;
        left = std::move(bin);
    }

    return left;
}

ExprPtr Parser::parseUnary() {
    if (match(TokenKind::Minus)) {
        auto operand = parseUnary();
        auto un = std::make_unique<UnaryExpr>(UnaryExpr::Negate, std::move(operand));
        un->line = previous().line;
        un->col = previous().col;
        return un;
    }
    return parseCall();
}

ExprPtr Parser::parseCall() {
    auto expr = parsePrimary();

    // Check if followed by '(' → function call
    if (check(TokenKind::LParen)) {
        // The primary must be an identifier for a call
        if (auto* ident = dynamic_cast<IdentifierExpr*>(expr.get())) {
            std::string callee = ident->name;
            advance();  // consume '('
            auto args = parseArgList();
            expect(TokenKind::RParen, "Expected ')' after arguments");

            auto call = std::make_unique<CallExpr>(callee, std::move(args));
            call->line = ident->line;
            call->col = ident->col;
            return call;
        }
    }

    return expr;
}

ExprPtr Parser::parsePrimary() {
    // Number literal
    if (check(TokenKind::Integer)) {
        Token tok = advance();
        auto num = std::make_unique<NumberLiteral>(
            NumberLiteral::IntLit, std::stod(tok.value), tok.value);
        num->line = tok.line;
        num->col = tok.col;
        return num;
    }

    if (check(TokenKind::Float)) {
        Token tok = advance();
        auto num = std::make_unique<NumberLiteral>(
            NumberLiteral::FloatLit, std::stod(tok.value), tok.value);
        num->line = tok.line;
        num->col = tok.col;
        return num;
    }

    // Identifier (variable, constant like BLOCK, or function name for later call)
    if (check(TokenKind::Identifier) || check(TokenKind::KW_load) ||
        check(TokenKind::KW_store) || check(TokenKind::KW_program_id) ||
        check(TokenKind::KW_arange) || check(TokenKind::KW_reduce_sum) ||
        check(TokenKind::KW_constant) || check(TokenKind::KW_max)) {
        Token tok = advance();
        auto id = std::make_unique<IdentifierExpr>(tok.value);
        id->line = tok.line;
        id->col = tok.col;
        return id;
    }

    // Parenthesized expression
    if (match(TokenKind::LParen)) {
        auto expr = parseExpression();
        expect(TokenKind::RParen, "Expected ')'");
        return expr;
    }

    error("Expected expression");
    advance();  // skip bad token
    auto err = std::make_unique<NumberLiteral>(NumberLiteral::IntLit, 0, "0");
    return err;
}

std::vector<ExprPtr> Parser::parseArgList() {
    std::vector<ExprPtr> args;

    if (check(TokenKind::RParen)) return args;

    args.push_back(parseExpression());
    while (match(TokenKind::Comma)) {
        args.push_back(parseExpression());
    }

    return args;
}

} // namespace minitriton
