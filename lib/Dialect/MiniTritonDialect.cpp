//===- MiniTritonDialect.cpp - MiniTriton dialect implementation -----------===//
//
// Implements the mt dialect registration, type parsing/printing,
// and custom operation assembly formats.
//
//===----------------------------------------------------------------------===//

#include "MiniTriton/Dialect/MiniTritonDialect.h"

#include "mlir/IR/Builders.h"
#include "mlir/IR/OpImplementation.h"
#include "mlir/IR/DialectImplementation.h"
#include "llvm/ADT/TypeSwitch.h"

using namespace mlir;
using namespace minitriton::mt;

//===----------------------------------------------------------------------===//
// TableGen-generated definitions
//===----------------------------------------------------------------------===//

#include "MiniTriton/Dialect/MiniTritonDialect.cpp.inc"

#define GET_TYPEDEF_CLASSES
#include "MiniTriton/Dialect/MiniTritonTypes.cpp.inc"

#define GET_OP_CLASSES
#include "MiniTriton/Dialect/MiniTritonOps.cpp.inc"

//===----------------------------------------------------------------------===//
// Dialect initialization
//===----------------------------------------------------------------------===//

void MiniTritonDialect::initialize() {
    registerTypes();

    addOperations<
#define GET_OP_LIST
#include "MiniTriton/Dialect/MiniTritonOps.cpp.inc"
    >();
}

void MiniTritonDialect::registerTypes() {
    addTypes<
#define GET_TYPEDEF_LIST
#include "MiniTriton/Dialect/MiniTritonTypes.cpp.inc"
    >();
}

//===----------------------------------------------------------------------===//
// KernelOp — custom assembly format
//===----------------------------------------------------------------------===//

void KernelOp::build(OpBuilder &builder, OperationState &state,
                     StringRef name, FunctionType type,
                     ArrayRef<NamedAttribute> attrs) {
    state.addAttribute(SymbolTable::getSymbolAttrName(),
                       builder.getStringAttr(name));
    state.addAttribute(getFunctionTypeAttrName(state.name),
                       TypeAttr::get(type));
    state.attributes.append(attrs.begin(), attrs.end());
    state.addRegion();
}

/// @kernel @vector_add(%arg0: !mt.ptr<f32>, ...) { ... }
mlir::ParseResult KernelOp::parse(OpAsmParser &parser,
                                   OperationState &result) {
    auto buildFuncType =
        [](Builder &builder, ArrayRef<Type> argTypes, ArrayRef<Type>,
           function_interface_impl::VariadicFlag,
           std::string &) { return builder.getFunctionType(argTypes, {}); };

    return function_interface_impl::parseFunctionOp(
        parser, result, /*allowVariadic=*/false,
        getFunctionTypeAttrName(result.name), buildFuncType,
        getArgAttrsAttrName(result.name), getResAttrsAttrName(result.name));
}

void KernelOp::print(OpAsmPrinter &p) {
    function_interface_impl::printFunctionOp(
        p, *this, /*isVariadic=*/false, getFunctionTypeAttrName(),
        getArgAttrsAttrName(), getResAttrsAttrName());
}
