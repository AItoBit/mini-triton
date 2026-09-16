//===- MiniTritonDialect.td - MiniTriton dialect definition ----------------===//
//
// Defines the MiniTriton (mt) MLIR dialect for high-level GPU tensor operations.
//
//===----------------------------------------------------------------------===//

#ifndef MINITRITON_DIALECT_TD
#define MINITRITON_DIALECT_TD

include "mlir/IR/OpBase.td"

//===----------------------------------------------------------------------===//
// MiniTriton Dialect
//===----------------------------------------------------------------------===//

def MiniTriton_Dialect : Dialect {
    let name = "mt";
    let summary = "MiniTriton dialect for high-level GPU kernel operations";
    let description = [{
        The MiniTriton (mt) dialect represents high-level tensor and GPU
        operations before lowering to concrete GPU execution primitives.

        It provides operations like `mt.load`, `mt.store`, `mt.program_id`,
        and arithmetic on tensors that map to GPU kernel patterns.
    }];
    let cppNamespace = "::minitriton::mt";
    let useDefaultTypePrinterParser = 1;

    let extraClassDeclaration = [{
        /// Register all MiniTriton types.
        void registerTypes();
    }];
}

//===----------------------------------------------------------------------===//
// MiniTriton Types
//===----------------------------------------------------------------------===//

// Base class for mt types
class MiniTriton_Type<string name, string typeMnemonic, list<Trait> traits = []>
    : TypeDef<MiniTriton_Dialect, name, traits> {
    let mnemonic = typeMnemonic;
}

def MiniTriton_PointerType : MiniTriton_Type<"Pointer", "ptr"> {
    let summary = "Pointer to a memory location";
    let description = [{
        A pointer type representing a reference to GPU global memory.
        Example: `!mt.ptr<f32>` is a pointer to f32 data.
    }];
    let parameters = (ins "::mlir::Type":$pointeeType);
    let assemblyFormat = "`<` $pointeeType `>`";
}

def MiniTriton_TensorType : MiniTriton_Type<"Tensor", "tensor"> {
    let summary = "Fixed-size tensor for block-level operations";
    let description = [{
        A tensor type representing a block of data processed by a single
        program instance. Example: `!mt.tensor<256xf32>`.
    }];
    let parameters = (ins "int64_t":$size, "::mlir::Type":$elementType);
    let assemblyFormat = "`<` $size `x` $elementType `>`";
}

#endif // MINITRITON_DIALECT_TD
