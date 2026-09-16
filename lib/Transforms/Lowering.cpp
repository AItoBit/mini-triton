//===- Lowering.cpp - GPU lowering pipeline implementation ----------------===//
//
// This file implements the lowering from the high-level `mt` dialect
// down to NVVM and LLVM dialects for PTX generation.
//
//===----------------------------------------------------------------------===//

#include "MiniTriton/Transforms/Lowering.h"
#include "MiniTriton/Dialect/MiniTritonDialect.h"

#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/SCF/IR/SCF.h"
#include "mlir/Dialect/GPU/IR/GPUDialect.h"
#include "mlir/Dialect/Math/IR/Math.h"
#include "mlir/Dialect/MemRef/IR/MemRef.h"
#include "mlir/Dialect/Vector/IR/VectorOps.h"
#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "mlir/Dialect/LLVMIR/NVVMDialect.h"

#include "mlir/Pass/PassManager.h"
#include "mlir/Transforms/DialectConversion.h"
#include "mlir/Transforms/Passes.h"

using namespace mlir;
using namespace minitriton::mt;

namespace {

//===----------------------------------------------------------------------===//
// mt → arith/scf/gpu conversions
//===----------------------------------------------------------------------===//

struct ProgramIdLowering : public OpConversionPattern<ProgramIdOp> {
    using OpConversionPattern<ProgramIdOp>::OpConversionPattern;

    LogicalResult matchAndRewrite(ProgramIdOp op, OpAdaptor adaptor,
                                  ConversionPatternRewriter &rewriter) const override {
        gpu::Dimension dim = gpu::Dimension::x;
        if (op.getAxis() == 1) dim = gpu::Dimension::y;
        if (op.getAxis() == 2) dim = gpu::Dimension::z;

        rewriter.replaceOpWithNewOp<gpu::BlockIdOp>(op, rewriter.getIndexType(), dim);
        return success();
    }
};

struct AddLowering : public OpConversionPattern<AddOp> {
    using OpConversionPattern<AddOp>::OpConversionPattern;

    LogicalResult matchAndRewrite(AddOp op, OpAdaptor adaptor,
                                  ConversionPatternRewriter &rewriter) const override {
        Type type = adaptor.getLhs().getType();
        if (type.isIntOrIndex()) {
            rewriter.replaceOpWithNewOp<arith::AddIOp>(op, adaptor.getLhs(), adaptor.getRhs());
        } else {
            rewriter.replaceOpWithNewOp<arith::AddFOp>(op, adaptor.getLhs(), adaptor.getRhs());
        }
        return success();
    }
};

struct LoadLowering : public OpConversionPattern<LoadOp> {
    using OpConversionPattern<LoadOp>::OpConversionPattern;

    LogicalResult matchAndRewrite(LoadOp op, OpAdaptor adaptor,
                                  ConversionPatternRewriter &rewriter) const override {
        // In a full compiler, this emits a vectorized masked load or an scf.for loop.
        // For the skeleton, we replace it with a dummy memref.load loop abstraction.
        rewriter.eraseOp(op);
        return success();
    }
};

// ... More conversions would go here (Store, Mul, Sub, Cmp, Reduce, Max) ...

//===----------------------------------------------------------------------===//
// Pass wrapping
//===----------------------------------------------------------------------===//

struct ConvertMiniTritonToStandardPass
    : public PassWrapper<ConvertMiniTritonToStandardPass, OperationPass<ModuleOp>> {
    MLIR_DEFINE_EXPLICIT_INTERNAL_INLINE_TYPE_ID(ConvertMiniTritonToStandardPass)

    void getDependentDialects(DialectRegistry &registry) const override {
        registry.insert<arith::ArithDialect, scf::SCFDialect,
                        gpu::GPUDialect, math::MathDialect,
                        memref::MemRefDialect, vector::VectorDialect>();
    }

    void runOnOperation() override {
        ConversionTarget target(getContext());
        target.addLegalDialect<arith::ArithDialect, scf::SCFDialect,
                               gpu::GPUDialect, math::MathDialect,
                               memref::MemRefDialect, vector::VectorDialect>();

        target.addIllegalDialect<MiniTritonDialect>();

        RewritePatternSet patterns(&getContext());
        patterns.add<ProgramIdLowering>(&getContext());
        patterns.add<AddLowering>(&getContext());
        patterns.add<LoadLowering>(&getContext());

        if (failed(applyPartialConversion(getOperation(), target, std::move(patterns)))) {
            signalPassFailure();
        }
    }
};

} // namespace

//===----------------------------------------------------------------------===//
// Public API
//===----------------------------------------------------------------------===//

namespace minitriton {

void registerLoweringPasses() {
    PassRegistration<ConvertMiniTritonToStandardPass>(
        "convert-mt-to-std",
        "Convert MiniTriton dialect to std/arith/scf/gpu dialects");
}

bool runLoweringPipeline(mlir::ModuleOp module) {
    PassManager pm(module.getContext());
    mlir::applyPassManagerCLOptions(pm);

    // 1. Lower mt to arith/scf/gpu/vector
    pm.addPass(std::make_unique<ConvertMiniTritonToStandardPass>());

    // 2. Canonicalization, CSE, etc. (Optimizer Phase 10)
    pm.addPass(createCanonicalizerPass());
    pm.addPass(createCSEPass());

    // 3. Lower standard dialects to LLVM and NVVM
    // Missing passes provided by MLIR:
    // - createGpuToLLVMConversionPass()
    // - createArithToLLVMConversionPass()

    if (failed(pm.run(module))) {
        return false;
    }
    return true;
}

} // namespace minitriton
