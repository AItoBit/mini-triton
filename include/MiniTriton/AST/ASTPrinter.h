#pragma once

#include "MiniTriton/AST/AST.h"
#include <string>

namespace minitriton {

/// Pretty-prints an AST to a string with indentation.
class ASTPrinter : public ASTVisitor {
public:
    std::string print(Module& module);

    void visit(NumberLiteral& node) override;
    void visit(IdentifierExpr& node) override;
    void visit(BinaryExpr& node) override;
    void visit(UnaryExpr& node) override;
    void visit(CallExpr& node) override;
    void visit(AssignStmt& node) override;
    void visit(ExprStmt& node) override;
    void visit(ReturnStmt& node) override;
    void visit(FunctionDef& node) override;
    void visit(Module& node) override;

private:
    std::string result_;
    int indentLevel_ = 0;

    void indent();
    void writeLine(const std::string& text);
    void write(const std::string& text);
};

} // namespace minitriton
