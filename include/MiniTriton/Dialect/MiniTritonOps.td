//===- MiniTritonOps.td - MiniTriton operation definitions -----------------===//
//
// Defines all operations in the MiniTriton (mt) MLIR dialect.
//
//===----------------------------------------------------------------------===//

#ifndef MINITRITON_OPS_TD
#define MINITRITON_OPS_TD

include "MiniTritonDialect.td"
include "mlir/IR/OpBase.td"
include "mlir/IR/FunctionInterfaces.td"
include "mlir/IR/SymbolInterfaces.td"
include "mlir/Interfaces/SideEffectInterfaces.td"
include "mlir/Interfaces/CallInterfaces.td"

// Base class for all mt operations
class MiniTriton_Op<string mnemonic, list<Trait> traits = []>
    : Op<MiniTriton_Dialect, mnemonic, traits>;

//===----------------------------------------------------------------------===//
// Kernel Operation — top-level GPU kernel function
//===----------------------------------------------------------------------===//

def KernelOp : MiniTriton_Op<"kernel", [
    IsolatedFromAbove,
    FunctionOpInterface,
    Symbol
]> {
    let summary = "Define a GPU kernel function";
    let description = [{
        Defines a GPU kernel entry point. This is the top-level container
        for all operations within a single kernel.

        Example:
        ```mlir
        mt.kernel @vector_add(%A: !mt.ptr<f32>, %B: !mt.ptr<f32>,
                              %C: !mt.ptr<f32>, %N: i32) {
            ...
        }
        ```
    }];

    let arguments = (ins
        SymbolNameAttr:$sym_name,
        TypeAttrOf<FunctionType>:$function_type,
        OptionalAttr<DictArrayAttr>:$arg_attrs,
        OptionalAttr<DictArrayAttr>:$res_attrs
    );
    let regions = (region AnyRegion:$body);

    let builders = [
        OpBuilder<(ins
            "::llvm::StringRef":$name,
            "::mlir::FunctionType":$type,
            CArg<"::llvm::ArrayRef<::mlir::NamedAttribute>", "{}">:$attrs
        )>
    ];

    let extraClassDeclaration = [{
        // FunctionOpInterface methods
        ::mlir::Region *getCallableRegion() { return &getBody(); }
        ::llvm::ArrayRef<::mlir::Type> getArgumentTypes() {
            return getFunctionType().getInputs();
        }
        ::llvm::ArrayRef<::mlir::Type> getResultTypes() {
            return getFunctionType().getResults();
        }
    }];

    let hasCustomAssemblyFormat = 1;
}

//===----------------------------------------------------------------------===//
// Program ID — get block/program identifier
//===----------------------------------------------------------------------===//

def ProgramIdOp : MiniTriton_Op<"program_id", [Pure]> {
    let summary = "Get the program/block ID for a given axis";
    let description = [{
        Returns the block identifier for the specified axis.
        Axis 0 = x, 1 = y, 2 = z.

        ```mlir
        %pid = mt.program_id {axis = 0 : i32}
        ```
    }];
    let arguments = (ins I32Attr:$axis);
    let results = (outs I32:$result);
    let assemblyFormat = "attr-dict";
}

//===----------------------------------------------------------------------===//
// Constants
//===----------------------------------------------------------------------===//

def ConstantOp : MiniTriton_Op<"constant", [Pure]> {
    let summary = "Create a constant value";
    let arguments = (ins AnyAttr:$value);
    let results = (outs AnyType:$result);
    let assemblyFormat = "$value attr-dict `:` type($result)";
}

//===----------------------------------------------------------------------===//
// Arange — create a range vector
//===----------------------------------------------------------------------===//

def ArangeOp : MiniTriton_Op<"arange", [Pure]> {
    let summary = "Create a range vector [start, end)";
    let description = [{
        Creates a 1D tensor with values from start (inclusive) to end (exclusive).
        ```mlir
        %range = mt.arange {start = 0 : i32, end = 256 : i32} : !mt.tensor<256xi32>
        ```
    }];
    let arguments = (ins I32Attr:$start, I32Attr:$end);
    let results = (outs AnyType:$result);
    let assemblyFormat = "attr-dict `:` type($result)";
}

//===----------------------------------------------------------------------===//
// Arithmetic Operations
//===----------------------------------------------------------------------===//

def AddOp : MiniTriton_Op<"add", [Pure, SameOperandsAndResultType]> {
    let summary = "Element-wise addition";
    let arguments = (ins AnyType:$lhs, AnyType:$rhs);
    let results = (outs AnyType:$result);
    let assemblyFormat = "$lhs `,` $rhs attr-dict `:` type($result)";
}

def SubOp : MiniTriton_Op<"sub", [Pure, SameOperandsAndResultType]> {
    let summary = "Element-wise subtraction";
    let arguments = (ins AnyType:$lhs, AnyType:$rhs);
    let results = (outs AnyType:$result);
    let assemblyFormat = "$lhs `,` $rhs attr-dict `:` type($result)";
}

def MulOp : MiniTriton_Op<"mul", [Pure, SameOperandsAndResultType]> {
    let summary = "Element-wise multiplication";
    let arguments = (ins AnyType:$lhs, AnyType:$rhs);
    let results = (outs AnyType:$result);
    let assemblyFormat = "$lhs `,` $rhs attr-dict `:` type($result)";
}

def DivOp : MiniTriton_Op<"div", [Pure, SameOperandsAndResultType]> {
    let summary = "Element-wise division";
    let arguments = (ins AnyType:$lhs, AnyType:$rhs);
    let results = (outs AnyType:$result);
    let assemblyFormat = "$lhs `,` $rhs attr-dict `:` type($result)";
}

//===----------------------------------------------------------------------===//
// Comparison
//===----------------------------------------------------------------------===//

def CmpOp : MiniTriton_Op<"cmp", [Pure]> {
    let summary = "Element-wise comparison";
    let description = [{
        Compares two values element-wise. Returns a mask (tensor of i1).
        Predicate can be: eq, ne, lt, le, gt, ge.
    }];
    let arguments = (ins
        StrAttr:$predicate,
        AnyType:$lhs,
        AnyType:$rhs
    );
    let results = (outs AnyType:$result);
    let assemblyFormat = "$predicate `,` $lhs `,` $rhs attr-dict `:` type($lhs) `->` type($result)";
}

//===----------------------------------------------------------------------===//
// Memory Operations
//===----------------------------------------------------------------------===//

def LoadOp : MiniTriton_Op<"load"> {
    let summary = "Load data from global memory";
    let description = [{
        Loads a tensor of values from global memory at the given pointer + offsets.
        An optional mask controls which elements are loaded.

        ```mlir
        %data = mt.load %ptr, %offsets, %mask : !mt.ptr<f32>, !mt.tensor<256xi32>, !mt.tensor<256xi1> -> !mt.tensor<256xf32>
        ```
    }];
    let arguments = (ins
        AnyType:$ptr,
        AnyType:$offsets,
        Optional<AnyType>:$mask
    );
    let results = (outs AnyType:$result);
    let assemblyFormat = "$ptr `,` $offsets (`,` $mask^)? attr-dict `:` type($ptr) `,` type($offsets) (`,` type($mask)^)? `->` type($result)";
}

def StoreOp : MiniTriton_Op<"store"> {
    let summary = "Store data to global memory";
    let description = [{
        Stores a tensor of values to global memory at the given pointer + offsets.
        An optional mask controls which elements are stored.
    }];
    let arguments = (ins
        AnyType:$ptr,
        AnyType:$offsets,
        AnyType:$value,
        Optional<AnyType>:$mask
    );
    let assemblyFormat = "$ptr `,` $offsets `,` $value (`,` $mask^)? attr-dict `:` type($ptr) `,` type($offsets) `,` type($value) (`,` type($mask)^)?";
}

//===----------------------------------------------------------------------===//
// Reduction
//===----------------------------------------------------------------------===//

def ReduceOp : MiniTriton_Op<"reduce", [Pure]> {
    let summary = "Reduce a tensor along an axis";
    let description = [{
        Reduces a tensor using the specified operation (sum, max, min).
    }];
    let arguments = (ins
        AnyType:$input,
        StrAttr:$reduce_op
    );
    let results = (outs AnyType:$result);
    let assemblyFormat = "$reduce_op $input attr-dict `:` type($input) `->` type($result)";
}

//===----------------------------------------------------------------------===//
// Max
//===----------------------------------------------------------------------===//

def MaxOp : MiniTriton_Op<"max", [Pure, SameOperandsAndResultType]> {
    let summary = "Element-wise maximum";
    let arguments = (ins AnyType:$lhs, AnyType:$rhs);
    let results = (outs AnyType:$result);
    let assemblyFormat = "$lhs `,` $rhs attr-dict `:` type($result)";
}

//===----------------------------------------------------------------------===//
// Yield / Return
//===----------------------------------------------------------------------===//

def ReturnOp : MiniTriton_Op<"return", [Terminator]> {
    let summary = "Return from a kernel (terminator)";
    let arguments = (ins Variadic<AnyType>:$operands);
    let assemblyFormat = "($operands^ `:` type($operands))? attr-dict";
}

#endif // MINITRITON_OPS_TD
