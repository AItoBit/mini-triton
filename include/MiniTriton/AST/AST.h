#pragma once

#include "MiniTriton/AST/Types.h"
#include <string>
#include <vector>
#include <memory>

namespace minitriton {

// Forward declarations
class ASTVisitor;

// -----------------------------------------------------------------------
// Base Node
// -----------------------------------------------------------------------

class ASTNode {
public:
    int line = 0;
    int col = 0;
    virtual ~ASTNode() = default;
    virtual void accept(ASTVisitor& visitor) = 0;
};

using ASTNodePtr = std::unique_ptr<ASTNode>;

// -----------------------------------------------------------------------
// Expressions
// -----------------------------------------------------------------------

class Expr : public ASTNode {
public:
    std::shared_ptr<Type> resolvedType;  // Filled by type checker
};
using ExprPtr = std::unique_ptr<Expr>;

/// Numeric literal: 42, 3.14
class NumberLiteral : public Expr {
public:
    enum Kind { IntLit, FloatLit };
    Kind numKind;
    double value;
    std::string raw;  // original text

    NumberLiteral(Kind k, double v, const std::string& r)
        : numKind(k), value(v), raw(r) {}
    void accept(ASTVisitor& visitor) override;
};

/// Variable / identifier reference: x, A, N, BLOCK
class IdentifierExpr : public Expr {
public:
    std::string name;

    explicit IdentifierExpr(const std::string& n) : name(n) {}
    void accept(ASTVisitor& visitor) override;
};

/// Binary expression: a + b, offset < N
class BinaryExpr : public Expr {
public:
    enum Op {
        Add, Sub, Mul, Div,
        Less, Greater, LessEq, GreaterEq, Equal, NotEqual
    };
    Op op;
    ExprPtr left;
    ExprPtr right;

    BinaryExpr(Op o, ExprPtr l, ExprPtr r)
        : op(o), left(std::move(l)), right(std::move(r)) {}
    void accept(ASTVisitor& visitor) override;

    static const char* opToString(Op op);
};

/// Unary expression: -x
class UnaryExpr : public Expr {
public:
    enum Op { Negate };
    Op op;
    ExprPtr operand;

    UnaryExpr(Op o, ExprPtr operand)
        : op(o), operand(std::move(operand)) {}
    void accept(ASTVisitor& visitor) override;
};

/// Function/builtin call: load(A + offset, mask), program_id(0)
class CallExpr : public Expr {
public:
    std::string callee;
    std::vector<ExprPtr> args;

    CallExpr(const std::string& c, std::vector<ExprPtr> a)
        : callee(c), args(std::move(a)) {}
    void accept(ASTVisitor& visitor) override;
};

// -----------------------------------------------------------------------
// Statements
// -----------------------------------------------------------------------

class Stmt : public ASTNode {};
using StmtPtr = std::unique_ptr<Stmt>;

/// Assignment: x = expr
class AssignStmt : public Stmt {
public:
    std::string target;
    ExprPtr value;

    AssignStmt(const std::string& t, ExprPtr v)
        : target(t), value(std::move(v)) {}
    void accept(ASTVisitor& visitor) override;
};

/// Expression statement: store(...)
class ExprStmt : public Stmt {
public:
    ExprPtr expr;

    explicit ExprStmt(ExprPtr e) : expr(std::move(e)) {}
    void accept(ASTVisitor& visitor) override;
};

/// Return statement: return expr
class ReturnStmt : public Stmt {
public:
    ExprPtr value;  // may be nullptr

    explicit ReturnStmt(ExprPtr v = nullptr) : value(std::move(v)) {}
    void accept(ASTVisitor& visitor) override;
};

// -----------------------------------------------------------------------
// Top-level
// -----------------------------------------------------------------------

/// Parameter: A: ptr<f32>
class Parameter {
public:
    std::string name;
    std::shared_ptr<Type> type;
    int line = 0;
    int col = 0;

    Parameter(const std::string& n, std::shared_ptr<Type> t)
        : name(n), type(std::move(t)) {}
};

/// Function definition: @kernel def vector_add(...)
class FunctionDef : public ASTNode {
public:
    std::string name;
    std::vector<Parameter> params;
    std::vector<StmtPtr> body;
    bool isKernel = false;

    FunctionDef(const std::string& n, std::vector<Parameter> p,
                std::vector<StmtPtr> b, bool kernel = false)
        : name(n), params(std::move(p)), body(std::move(b)), isKernel(kernel) {}
    void accept(ASTVisitor& visitor) override;
};

/// Module: top-level container of functions
class Module : public ASTNode {
public:
    std::vector<std::unique_ptr<FunctionDef>> functions;

    void accept(ASTVisitor& visitor) override;
};

// -----------------------------------------------------------------------
// Visitor
// -----------------------------------------------------------------------

class ASTVisitor {
public:
    virtual ~ASTVisitor() = default;

    virtual void visit(NumberLiteral& node) = 0;
    virtual void visit(IdentifierExpr& node) = 0;
    virtual void visit(BinaryExpr& node) = 0;
    virtual void visit(UnaryExpr& node) = 0;
    virtual void visit(CallExpr& node) = 0;
    virtual void visit(AssignStmt& node) = 0;
    virtual void visit(ExprStmt& node) = 0;
    virtual void visit(ReturnStmt& node) = 0;
    virtual void visit(FunctionDef& node) = 0;
    virtual void visit(Module& node) = 0;
};

} // namespace minitriton
