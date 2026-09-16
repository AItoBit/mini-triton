//===- Lowering.h - GPU lowering pipeline ---------------------------------===//
//
// Defines the lowering pipeline from MiniTriton dialect to LLVM IR/PTX.
//
//===----------------------------------------------------------------------===//

#pragma once

#include "mlir/IR/BuiltinOps.h"
#include "mlir/Pass/PassManager.h"
#include <memory>

namespace minitriton {

/// Run the full lowering pipeline on a module.
/// 1. mt → arith, scf, gpu, math
/// 2. Vectorization / Tiling
/// 3. std → llvm, gpu → nvvm
/// Returns false on failure.
bool runLoweringPipeline(mlir::ModuleOp module);

/// Register all MiniTriton lowering passes with the MLIR registry.
void registerLoweringPasses();

} // namespace minitriton
