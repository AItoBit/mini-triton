//===- MiniTritonDialect.h - MiniTriton dialect declaration ----------------===//
//
// C++ declaration for the mt MLIR dialect.
//
//===----------------------------------------------------------------------===//

#pragma once

#include "mlir/IR/Dialect.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/BuiltinTypes.h"
#include "mlir/IR/FunctionInterfaces.h"
#include "mlir/IR/SymbolTable.h"
#include "mlir/Interfaces/CallInterfaces.h"
#include "mlir/Interfaces/SideEffectInterfaces.h"

// Include TableGen-generated dialect declaration
#include "MiniTriton/Dialect/MiniTritonDialect.h.inc"

namespace minitriton {
namespace mt {

//===----------------------------------------------------------------------===//
// MiniTriton Types (C++ side)
//===----------------------------------------------------------------------===//

// Include TableGen-generated type declarations
#define GET_TYPEDEF_CLASSES
#include "MiniTriton/Dialect/MiniTritonTypes.h.inc"

//===----------------------------------------------------------------------===//
// MiniTriton Operations (C++ side)
//===----------------------------------------------------------------------===//

// Include TableGen-generated op declarations
#define GET_OP_CLASSES
#include "MiniTriton/Dialect/MiniTritonOps.h.inc"

} // namespace mt
} // namespace minitriton
