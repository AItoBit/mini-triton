#pragma once

#include "MiniTriton/AST/AST.h"
#include <string>
#include <vector>
#include <unordered_map>

namespace minitriton {

/// Semantic analysis and type checking pass over the AST.
class TypeChecker : public ASTVisitor {
public:
    /// Run type checking on a module. Returns true if no errors.
    bool check(Module& module);

    /// Get collected errors.
    const std::vector<std::string>& getErrors() const { return errors_; }

    // Visitor methods
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
    std::vector<std::string> errors_;

    // Symbol table: variable name → type
    std::unordered_map<std::string, std::shared_ptr<Type>> symbolTable_;

    // Current expression result type (set by expression visitors)
    std::shared_ptr<Type> currentType_;

    // Error reporting
    void error(int line, int col, const std::string& message);

    // Type helpers
    void resolveExprType(Expr& expr);
    bool isCompatible(const std::shared_ptr<Type>& a, const std::shared_ptr<Type>& b) const;
    std::shared_ptr<Type> resultType(BinaryExpr::Op op,
                                      const std::shared_ptr<Type>& left,
                                      const std::shared_ptr<Type>& right) const;
};

} // namespace minitriton
