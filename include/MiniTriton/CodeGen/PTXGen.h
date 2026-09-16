//===- PTXGen.h - Generic PTX binary generation -----------------------------===//
//
// Emits PTX assembly from an LLVM module via the NVPTX backend.
//
//===----------------------------------------------------------------------===//

#pragma once

#include "mlir/IR/BuiltinOps.h"
#include <string>

namespace minitriton {

/// Translate an MLIR module (already in LLVM/NVVM dialect) to PTX assembly.
/// Returns empty string on failure.
std::string translateMLIRToPTX(mlir::ModuleOp module);

} // namespace minitriton
