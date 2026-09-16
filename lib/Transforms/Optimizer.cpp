//===- Optimizer.cpp - Optimizer implementation -----------------------------===//
//
// M10: Canonicalization, DCE, and other high-level passes.
//
//===----------------------------------------------------------------------===//

#include "MiniTriton/Transforms/Optimizer.h"
#include "mlir/Pass/PassManager.h"
#include "mlir/Transforms/Passes.h"

using namespace mlir;

namespace minitriton {

bool runOptimizationPipeline(mlir::ModuleOp module) {
    PassManager pm(module.getContext());
    mlir::applyPassManagerCLOptions(pm);

    // Apply high-level optimization passes (M10)
    pm.addPass(createCanonicalizerPass());
    pm.addPass(createCSEPass());
    pm.addPass(createSymbolDCEPass());

    if (failed(pm.run(module))) {
        return false;
    }
    return true;
}

} // namespace minitriton
