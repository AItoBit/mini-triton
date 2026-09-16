#include "MiniTriton/AST/AST.h"
#include "MiniTriton/AST/Types.h"

namespace minitriton {

// -----------------------------------------------------------------------
// Type
// -----------------------------------------------------------------------

bool Type::operator==(const Type& other) const {
    if (kind != other.kind) return false;
    if (kind == TypeKind::Pointer || kind == TypeKind::Tensor) {
        if (!elementType && !other.elementType) return true;
        if (!elementType || !other.elementType) return false;
        return *elementType == *other.elementType;
    }
    return true;
}

std::string Type::toString() const {
    switch (kind) {
        case TypeKind::I32:     return "i32";
        case TypeKind::F32:     return "f32";
        case TypeKind::Bool:    return "bool";
        case TypeKind::Void:    return "void";
        case TypeKind::Unknown: return "unknown";
        case TypeKind::Pointer:
            return "ptr<" + (elementType ? elementType->toString() : "?") + ">";
        case TypeKind::Tensor:
            return "tensor<" + (elementType ? elementType->toString() : "?") +
                   (size >= 0 ? ", " + std::to_string(size) : "") + ">";
    }
    return "?";
}

// -----------------------------------------------------------------------
// AST Accept methods
// -----------------------------------------------------------------------

void NumberLiteral::accept(ASTVisitor& v) { v.visit(*this); }
void IdentifierExpr::accept(ASTVisitor& v) { v.visit(*this); }
void BinaryExpr::accept(ASTVisitor& v) { v.visit(*this); }
void UnaryExpr::accept(ASTVisitor& v) { v.visit(*this); }
void CallExpr::accept(ASTVisitor& v) { v.visit(*this); }
void AssignStmt::accept(ASTVisitor& v) { v.visit(*this); }
void ExprStmt::accept(ASTVisitor& v) { v.visit(*this); }
void ReturnStmt::accept(ASTVisitor& v) { v.visit(*this); }
void FunctionDef::accept(ASTVisitor& v) { v.visit(*this); }
void Module::accept(ASTVisitor& v) { v.visit(*this); }

// -----------------------------------------------------------------------
// BinaryExpr::opToString
// -----------------------------------------------------------------------

const char* BinaryExpr::opToString(Op op) {
    switch (op) {
        case Add:       return "+";
        case Sub:       return "-";
        case Mul:       return "*";
        case Div:       return "/";
        case Less:      return "<";
        case Greater:   return ">";
        case LessEq:    return "<=";
        case GreaterEq: return ">=";
        case Equal:     return "==";
        case NotEqual:  return "!=";
    }
    return "?";
}

} // namespace minitriton
