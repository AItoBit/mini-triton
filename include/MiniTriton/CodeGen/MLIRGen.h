//===- MLIRGen.h - AST to MLIR generation ---------------------------------===//
//
// Converts MiniTriton AST to mt dialect MLIR.
//
//===----------------------------------------------------------------------===//

#pragma once

#include "MiniTriton/AST/AST.h"
#include "mlir/IR/MLIRContext.h"
#include "mlir/IR/BuiltinOps.h"
#include <memory>

namespace minitriton {

/// Generate MLIR (in the mt dialect) from a MiniTriton AST Module.
/// Returns nullptr on failure.
mlir::OwningOpRef<mlir::ModuleOp> mlirGen(mlir::MLIRContext &context,
                                           Module &astModule);

} // namespace minitriton
