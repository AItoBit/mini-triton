//===- Optimizer.h - MiniTriton high-level optimizer ----------------------===//
//
// Implements backend optimization passes (M10) like canonicalization and DCE.
//
//===----------------------------------------------------------------------===//

#pragma once

#include "mlir/IR/BuiltinOps.h"
#include <memory>

namespace minitriton {

/// Run high-level mt dialect optimizations before lowering.
/// This includes constant folding, common subexpression elimination,
/// and dead code elimination.
bool runOptimizationPipeline(mlir::ModuleOp module);

} // namespace minitriton
