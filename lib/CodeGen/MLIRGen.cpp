//===- MLIRGen.cpp - AST to MiniTriton MLIR generation --------------------===//
//
// Walks the MiniTriton AST and emits corresponding mt dialect operations.
//
// This is the bridge from the frontend (M1-M2) to the MLIR world (M3+).
// Each AST node maps to one or more mt.* operations.
//
//===----------------------------------------------------------------------===//

#include "MiniTriton/CodeGen/MLIRGen.h"
#include "MiniTriton/Dialect/MiniTritonDialect.h"

#include "mlir/IR/Builders.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/MLIRContext.h"
#include "mlir/IR/Verifier.h"

#include "llvm/ADT/ScopedHashTable.h"
#include "llvm/ADT/StringRef.h"

#include <unordered_map>

using namespace mlir;
using namespace minitriton;

namespace {

// -----------------------------------------------------------------------
// MLIR Generator — walks AST and emits mt dialect ops
// -----------------------------------------------------------------------

class MLIRGenImpl : public ASTVisitor {
public:
    MLIRGenImpl(MLIRContext &context)
        : builder_(&context), context_(context) {}

    /// Generate a full MLIR module from the AST.
    OwningOpRef<ModuleOp> generate(Module &astModule) {
        mlirModule_ = ModuleOp::create(builder_.getUnknownLoc());
        builder_.setInsertionPointToEnd(mlirModule_->getBody());

        for (auto &fn : astModule.functions) {
            fn->accept(*this);
        }

        if (failed(verify(*mlirModule_))) {
            mlirModule_->emitError("MLIR module verification failed");
            return nullptr;
        }

        return std::move(mlirModule_);
    }

    // --- Visitor methods ---

    void visit(Module &node) override {
        for (auto &fn : node.functions)
            fn->accept(*this);
    }

    void visit(FunctionDef &node) override {
        // Build parameter types
        llvm::SmallVector<Type, 8> argTypes;
        for (auto &param : node.params) {
            argTypes.push_back(convertType(param.type));
        }

        auto funcType = builder_.getFunctionType(argTypes, {});
        auto loc = getLoc(node.line, node.col);

        // Create mt.kernel op
        auto kernelOp = builder_.create<mt::KernelOp>(loc, node.name, funcType);

        // Create entry block with arguments
        auto &entryBlock = *kernelOp.addEntryBlock();
        builder_.setInsertionPointToStart(&entryBlock);

        // Register parameters in symbol table
        symbolTable_.clear();
        for (size_t i = 0; i < node.params.size(); i++) {
            symbolTable_[node.params[i].name] = entryBlock.getArgument(i);
        }

        // Generate body
        for (auto &stmt : node.body) {
            stmt->accept(*this);
        }

        // Add terminator if needed
        if (entryBlock.empty() ||
            !entryBlock.back().hasTrait<OpTrait::IsTerminator>()) {
            builder_.create<mt::ReturnOp>(getLoc(node.line, node.col));
        }

        // Reset insertion point to module level
        builder_.setInsertionPointToEnd(mlirModule_->getBody());
    }

    void visit(AssignStmt &node) override {
        node.value->accept(*this);
        symbolTable_[node.target] = lastValue_;
    }

    void visit(ExprStmt &node) override {
        node.expr->accept(*this);
    }

    void visit(ReturnStmt &node) override {
        auto loc = getLoc(node.line, node.col);
        if (node.value) {
            node.value->accept(*this);
            builder_.create<mt::ReturnOp>(loc, ValueRange{lastValue_});
        } else {
            builder_.create<mt::ReturnOp>(loc);
        }
    }

    void visit(NumberLiteral &node) override {
        auto loc = getLoc(node.line, node.col);
        if (node.numKind == NumberLiteral::FloatLit) {
            auto type = builder_.getF32Type();
            auto attr = builder_.getF32FloatAttr(static_cast<float>(node.value));
            lastValue_ = builder_.create<mt::ConstantOp>(loc, type, attr);
        } else {
            auto type = builder_.getI32Type();
            auto attr = builder_.getI32IntegerAttr(static_cast<int>(node.value));
            lastValue_ = builder_.create<mt::ConstantOp>(loc, type, attr);
        }
    }

    void visit(IdentifierExpr &node) override {
        auto loc = getLoc(node.line, node.col);

        // Check for compile-time constants
        if (node.name == "BLOCK" || node.name == "BLOCK_K") {
            int blockSize = (node.name == "BLOCK") ? 256 : 64;
            auto attr = builder_.getI32IntegerAttr(blockSize);
            lastValue_ = builder_.create<mt::ConstantOp>(
                loc, builder_.getI32Type(), attr);
            return;
        }

        auto it = symbolTable_.find(node.name);
        if (it != symbolTable_.end()) {
            lastValue_ = it->second;
        } else {
            emitError(loc, "Undefined variable: " + node.name);
        }
    }

    void visit(BinaryExpr &node) override {
        auto loc = getLoc(node.line, node.col);

        node.left->accept(*this);
        Value lhs = lastValue_;

        node.right->accept(*this);
        Value rhs = lastValue_;

        // Determine result type (use lhs type, or tensor if either is tensor)
        Type resultType = lhs.getType();
        if (auto tt = rhs.getType().dyn_cast<mt::TensorType>()) {
            resultType = rhs.getType();
        }

        switch (node.op) {
            case BinaryExpr::Add:
                lastValue_ = builder_.create<mt::AddOp>(loc, resultType, lhs, rhs);
                break;
            case BinaryExpr::Sub:
                lastValue_ = builder_.create<mt::SubOp>(loc, resultType, lhs, rhs);
                break;
            case BinaryExpr::Mul:
                lastValue_ = builder_.create<mt::MulOp>(loc, resultType, lhs, rhs);
                break;
            case BinaryExpr::Div:
                lastValue_ = builder_.create<mt::DivOp>(loc, resultType, lhs, rhs);
                break;
            case BinaryExpr::Less:
                lastValue_ = builder_.create<mt::CmpOp>(
                    loc, getMaskType(resultType),
                    builder_.getStringAttr("lt"), lhs, rhs);
                break;
            case BinaryExpr::Greater:
                lastValue_ = builder_.create<mt::CmpOp>(
                    loc, getMaskType(resultType),
                    builder_.getStringAttr("gt"), lhs, rhs);
                break;
            case BinaryExpr::LessEq:
                lastValue_ = builder_.create<mt::CmpOp>(
                    loc, getMaskType(resultType),
                    builder_.getStringAttr("le"), lhs, rhs);
                break;
            case BinaryExpr::GreaterEq:
                lastValue_ = builder_.create<mt::CmpOp>(
                    loc, getMaskType(resultType),
                    builder_.getStringAttr("ge"), lhs, rhs);
                break;
            case BinaryExpr::Equal:
                lastValue_ = builder_.create<mt::CmpOp>(
                    loc, getMaskType(resultType),
                    builder_.getStringAttr("eq"), lhs, rhs);
                break;
            case BinaryExpr::NotEqual:
                lastValue_ = builder_.create<mt::CmpOp>(
                    loc, getMaskType(resultType),
                    builder_.getStringAttr("ne"), lhs, rhs);
                break;
        }
    }

    void visit(UnaryExpr &node) override {
        auto loc = getLoc(node.line, node.col);
        node.operand->accept(*this);
        Value operand = lastValue_;

        // Negate: 0 - operand
        Type type = operand.getType();
        Value zero;
        if (type.isF32()) {
            zero = builder_.create<mt::ConstantOp>(
                loc, type, builder_.getF32FloatAttr(0.0f));
        } else {
            zero = builder_.create<mt::ConstantOp>(
                loc, type, builder_.getI32IntegerAttr(0));
        }
        lastValue_ = builder_.create<mt::SubOp>(loc, type, zero, operand);
    }

    void visit(CallExpr &node) override {
        auto loc = getLoc(node.line, node.col);

        // Evaluate all arguments
        llvm::SmallVector<Value, 4> args;
        for (auto &arg : node.args) {
            arg->accept(*this);
            args.push_back(lastValue_);
        }

        // --- Builtins ---

        if (node.callee == "program_id") {
            int axis = 0;
            if (!args.empty()) {
                if (auto constOp = args[0].getDefiningOp<mt::ConstantOp>()) {
                    if (auto intAttr = constOp.getValue().dyn_cast<IntegerAttr>()) {
                        axis = intAttr.getInt();
                    }
                }
            }
            lastValue_ = builder_.create<mt::ProgramIdOp>(
                loc, builder_.getI32Type(),
                builder_.getI32IntegerAttr(axis));
            return;
        }

        if (node.callee == "arange") {
            int start = 0, end = 256;
            if (args.size() >= 2) {
                if (auto c = args[0].getDefiningOp<mt::ConstantOp>())
                    if (auto a = c.getValue().dyn_cast<IntegerAttr>())
                        start = a.getInt();
                if (auto c = args[1].getDefiningOp<mt::ConstantOp>())
                    if (auto a = c.getValue().dyn_cast<IntegerAttr>())
                        end = a.getInt();
            }
            int size = end - start;
            auto tensorType = mt::TensorType::get(&context_, size,
                                                   builder_.getI32Type());
            lastValue_ = builder_.create<mt::ArangeOp>(
                loc, tensorType,
                builder_.getI32IntegerAttr(start),
                builder_.getI32IntegerAttr(end));
            return;
        }

        if (node.callee == "load") {
            Value ptr = args[0];
            Value offsets = (args.size() > 1) ? args[1] : ptr;
            Value mask = (args.size() > 2) ? args[2] : Value();

            // Determine result type: tensor of element type pointed to
            Type resultType;
            if (auto ptrType = ptr.getType().dyn_cast<mt::PointerType>()) {
                // Get size from offsets if it's a tensor
                int64_t size = 256;
                if (auto tt = offsets.getType().dyn_cast<mt::TensorType>())
                    size = tt.getSize();
                resultType = mt::TensorType::get(&context_, size,
                                                  ptrType.getPointeeType());
            } else {
                resultType = mt::TensorType::get(&context_, 256,
                                                  builder_.getF32Type());
            }

            lastValue_ = builder_.create<mt::LoadOp>(
                loc, resultType, ptr, offsets, mask);
            return;
        }

        if (node.callee == "store") {
            Value ptr = args[0];
            Value offsets = (args.size() > 1) ? args[1] : ptr;
            Value value = (args.size() > 2) ? args[2] : args[1];
            Value mask = (args.size() > 3) ? args[3] : Value();

            builder_.create<mt::StoreOp>(loc, ptr, offsets, value, mask);
            lastValue_ = Value();
            return;
        }

        if (node.callee == "reduce_sum") {
            Value input = args[0];
            Type elemType = builder_.getF32Type();
            if (auto tt = input.getType().dyn_cast<mt::TensorType>())
                elemType = tt.getElementType();

            lastValue_ = builder_.create<mt::ReduceOp>(
                loc, elemType, input, builder_.getStringAttr("sum"));
            return;
        }

        if (node.callee == "constant") {
            // Already handled as a literal in most cases, but handle explicit
            lastValue_ = args[0];
            return;
        }

        if (node.callee == "max") {
            Type resultType = args[0].getType();
            lastValue_ = builder_.create<mt::MaxOp>(
                loc, resultType, args[0], args[1]);
            return;
        }

        emitError(loc, "Unknown function call: " + node.callee);
    }

private:
    OpBuilder builder_;
    MLIRContext &context_;
    OwningOpRef<ModuleOp> mlirModule_;

    // Symbol table: variable name → MLIR Value
    std::unordered_map<std::string, Value> symbolTable_;

    // Last expression result value
    Value lastValue_;

    Location getLoc(int line, int col) {
        return FileLineColLoc::get(
            builder_.getStringAttr("<minitriton>"), line, col);
    }

    /// Convert frontend Type to MLIR Type.
    Type convertType(const std::shared_ptr<minitriton::Type> &astType) {
        if (!astType) return builder_.getNoneType();

        switch (astType->kind) {
            case TypeKind::I32:
                return builder_.getI32Type();
            case TypeKind::F32:
                return builder_.getF32Type();
            case TypeKind::Bool:
                return builder_.getI1Type();
            case TypeKind::Pointer:
                return mt::PointerType::get(
                    &context_, convertType(astType->elementType));
            case TypeKind::Tensor: {
                int64_t size = (astType->size > 0) ? astType->size : 256;
                return mt::TensorType::get(
                    &context_, size, convertType(astType->elementType));
            }
            case TypeKind::Void:
                return builder_.getNoneType();
            default:
                return builder_.getF32Type();
        }
    }

    /// Get a mask type (tensor<NxI1>) for comparison results.
    Type getMaskType(Type operandType) {
        if (auto tt = operandType.dyn_cast<mt::TensorType>()) {
            return mt::TensorType::get(&context_, tt.getSize(),
                                        builder_.getI1Type());
        }
        return builder_.getI1Type();
    }
};

} // anonymous namespace

// -----------------------------------------------------------------------
// Public API
// -----------------------------------------------------------------------

OwningOpRef<ModuleOp> minitriton::mlirGen(MLIRContext &context,
                                           Module &astModule) {
    context.getOrLoadDialect<mt::MiniTritonDialect>();
    MLIRGenImpl gen(context);
    return gen.generate(astModule);
}
