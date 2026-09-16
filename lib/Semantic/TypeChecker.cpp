#include "MiniTriton/Semantic/TypeChecker.h"
#include <sstream>

namespace minitriton {

// -----------------------------------------------------------------------
// Error reporting
// -----------------------------------------------------------------------

void TypeChecker::error(int line, int col, const std::string& message) {
    std::ostringstream oss;
    oss << "type error at " << line << ":" << col << ": " << message;
    errors_.push_back(oss.str());
}

// -----------------------------------------------------------------------
// Public interface
// -----------------------------------------------------------------------

bool TypeChecker::check(Module& module) {
    errors_.clear();
    module.accept(*this);
    return errors_.empty();
}

// -----------------------------------------------------------------------
// Type helpers
// -----------------------------------------------------------------------

bool TypeChecker::isCompatible(const std::shared_ptr<Type>& a,
                                const std::shared_ptr<Type>& b) const {
    if (!a || !b) return true;  // unknown types are compatible with anything
    if (a->kind == TypeKind::Unknown || b->kind == TypeKind::Unknown) return true;

    // Same kind
    if (a->kind == b->kind) return true;

    // i32 and f32 are compatible for arithmetic (implicit promotion)
    if (a->isNumeric() && b->isNumeric()) return true;

    // Pointer + i32/tensor for address arithmetic
    if ((a->isPointer() && b->isNumeric()) || (a->isNumeric() && b->isPointer()))
        return true;
    if ((a->isPointer() && b->isTensor()) || (a->isTensor() && b->isPointer()))
        return true;

    // Tensor types are compatible if element types are
    if (a->isTensor() && b->isTensor()) {
        return isCompatible(a->elementType, b->elementType);
    }

    return false;
}

std::shared_ptr<Type> TypeChecker::resultType(BinaryExpr::Op op,
                                               const std::shared_ptr<Type>& left,
                                               const std::shared_ptr<Type>& right) const {
    // Comparison operations return bool
    if (op == BinaryExpr::Less || op == BinaryExpr::Greater ||
        op == BinaryExpr::LessEq || op == BinaryExpr::GreaterEq ||
        op == BinaryExpr::Equal || op == BinaryExpr::NotEqual) {
        // If either operand is a tensor, result is tensor of bools (mask)
        if (left && left->isTensor()) return Type::makeTensor(Type::makeBool());
        if (right && right->isTensor()) return Type::makeTensor(Type::makeBool());
        return Type::makeBool();
    }

    // Pointer arithmetic: ptr + int → ptr
    if (left && left->isPointer()) return left;
    if (right && right->isPointer()) return right;

    // Tensor arithmetic: tensor + tensor → tensor, tensor + scalar → tensor
    if (left && left->isTensor()) return left;
    if (right && right->isTensor()) return right;

    // f32 promotion
    if ((left && left->kind == TypeKind::F32) || (right && right->kind == TypeKind::F32))
        return Type::makeF32();

    return Type::makeI32();
}

// -----------------------------------------------------------------------
// Visitor methods — Expressions
// -----------------------------------------------------------------------

void TypeChecker::visit(NumberLiteral& node) {
    if (node.numKind == NumberLiteral::FloatLit) {
        node.resolvedType = Type::makeF32();
    } else {
        node.resolvedType = Type::makeI32();
    }
    currentType_ = node.resolvedType;
}

void TypeChecker::visit(IdentifierExpr& node) {
    auto it = symbolTable_.find(node.name);
    if (it != symbolTable_.end()) {
        node.resolvedType = it->second;
    } else {
        // Allow unknown identifiers as compile-time constants (BLOCK, BLOCK_K)
        if (node.name == "BLOCK" || node.name == "BLOCK_K") {
            node.resolvedType = Type::makeI32();
        } else {
            error(node.line, node.col, "Undefined variable '" + node.name + "'");
            node.resolvedType = std::make_shared<Type>(TypeKind::Unknown);
        }
    }
    currentType_ = node.resolvedType;
}

void TypeChecker::visit(BinaryExpr& node) {
    node.left->accept(*this);
    auto leftType = currentType_;

    node.right->accept(*this);
    auto rightType = currentType_;

    // Check operand compatibility
    if (!isCompatible(leftType, rightType)) {
        error(node.line, node.col,
              "Incompatible types in binary expression: " +
              (leftType ? leftType->toString() : "unknown") + " " +
              BinaryExpr::opToString(node.op) + " " +
              (rightType ? rightType->toString() : "unknown"));
    }

    // Specific check: pointer + pointer is not allowed
    if (leftType && rightType && leftType->isPointer() && rightType->isPointer() &&
        node.op != BinaryExpr::Equal && node.op != BinaryExpr::NotEqual) {
        error(node.line, node.col, "Cannot perform arithmetic on two pointers");
    }

    node.resolvedType = resultType(node.op, leftType, rightType);
    currentType_ = node.resolvedType;
}

void TypeChecker::visit(UnaryExpr& node) {
    node.operand->accept(*this);
    auto operandType = currentType_;

    if (operandType && !operandType->isNumeric() && !operandType->isTensor()) {
        error(node.line, node.col,
              "Cannot negate non-numeric type: " + operandType->toString());
    }

    node.resolvedType = operandType;
    currentType_ = node.resolvedType;
}

void TypeChecker::visit(CallExpr& node) {
    // Type-check arguments first
    std::vector<std::shared_ptr<Type>> argTypes;
    for (auto& arg : node.args) {
        arg->accept(*this);
        argTypes.push_back(currentType_);
    }

    // Check builtin function signatures
    if (node.callee == "program_id") {
        if (argTypes.size() != 1) {
            error(node.line, node.col, "program_id expects 1 argument (axis)");
        }
        node.resolvedType = Type::makeI32();
    }
    else if (node.callee == "arange") {
        if (argTypes.size() != 2) {
            error(node.line, node.col, "arange expects 2 arguments (start, end)");
        }
        node.resolvedType = Type::makeTensor(Type::makeI32());
    }
    else if (node.callee == "load") {
        if (argTypes.size() < 1 || argTypes.size() > 2) {
            error(node.line, node.col, "load expects 1-2 arguments (ptr[, mask])");
        }
        if (!argTypes.empty() && argTypes[0] && !argTypes[0]->isPointer()) {
            // Allow pointer arithmetic result (which stays pointer type)
            // Actually, the first arg after ptr + offset gets pointer type from our rules
        }
        // Result is tensor of the pointed-to type
        if (!argTypes.empty() && argTypes[0] && argTypes[0]->isPointer() &&
            argTypes[0]->elementType) {
            node.resolvedType = Type::makeTensor(argTypes[0]->elementType);
        } else {
            node.resolvedType = Type::makeTensor(Type::makeF32());
        }
    }
    else if (node.callee == "store") {
        if (argTypes.size() < 2 || argTypes.size() > 3) {
            error(node.line, node.col, "store expects 2-3 arguments (ptr, value[, mask])");
        }
        if (!argTypes.empty() && argTypes[0] && !argTypes[0]->isPointer()) {
            error(node.line, node.col,
                  "First argument to store must be a pointer, got " +
                  argTypes[0]->toString());
        }
        node.resolvedType = Type::makeVoid();
    }
    else if (node.callee == "reduce_sum") {
        if (argTypes.size() != 1) {
            error(node.line, node.col, "reduce_sum expects 1 argument");
        }
        if (!argTypes.empty() && argTypes[0] && argTypes[0]->isTensor() &&
            argTypes[0]->elementType) {
            node.resolvedType = argTypes[0]->elementType;
        } else {
            node.resolvedType = Type::makeF32();
        }
    }
    else if (node.callee == "constant") {
        if (argTypes.size() != 1) {
            error(node.line, node.col, "constant expects 1 argument");
        }
        if (!argTypes.empty() && argTypes[0]) {
            node.resolvedType = argTypes[0];
        } else {
            node.resolvedType = Type::makeF32();
        }
    }
    else if (node.callee == "max") {
        if (argTypes.size() != 2) {
            error(node.line, node.col, "max expects 2 arguments");
        }
        // Result type is the "wider" of the two
        if (!argTypes.empty() && argTypes[0]) {
            node.resolvedType = argTypes[0];
        } else {
            node.resolvedType = Type::makeF32();
        }
    }
    else {
        // Unknown function — could be user-defined later
        error(node.line, node.col, "Unknown function '" + node.callee + "'");
        node.resolvedType = std::make_shared<Type>(TypeKind::Unknown);
    }

    currentType_ = node.resolvedType;
}

// -----------------------------------------------------------------------
// Visitor methods — Statements
// -----------------------------------------------------------------------

void TypeChecker::visit(AssignStmt& node) {
    node.value->accept(*this);
    symbolTable_[node.target] = currentType_;
}

void TypeChecker::visit(ExprStmt& node) {
    node.expr->accept(*this);
}

void TypeChecker::visit(ReturnStmt& node) {
    if (node.value) {
        node.value->accept(*this);
    }
}

// -----------------------------------------------------------------------
// Visitor methods — Top-level
// -----------------------------------------------------------------------

void TypeChecker::visit(FunctionDef& node) {
    // Clear symbol table for each function
    symbolTable_.clear();

    // Register parameters
    for (auto& param : node.params) {
        symbolTable_[param.name] = param.type;
    }

    // Check body
    for (auto& stmt : node.body) {
        stmt->accept(*this);
    }
}

void TypeChecker::visit(Module& node) {
    for (auto& fn : node.functions) {
        fn->accept(*this);
    }
}

} // namespace minitriton
