# MiniTriton Compiler Architecture

MiniTriton is a 6-layer compiler stack designed to translate a Python-like high-level language into highly optimized GPU PTX assembly.

## 1. Frontend (AST generation)
The frontend parses `.mt` files utilizing an indentation-aware lexer and a recursive-descent parser.
- **Lexer**: `Token`, `Lexer` tracking `INDENT/DEDENT`
- **Parser**: Hand-rolled, precedence climbing for expressions.
- **AST**: Object-oriented node tree (`Expr`, `Stmt`, `Block`).

## 2. Semantic Analysis
The `TypeChecker` walks the AST ensuring:
1. All variables are declared and live in scope.
2. Type signatures for binary ops and builtin calls (`load`, `store`, `program_id`) are compatible.
3. Propagates tensor sizes and element types.

## 3. MLIR CodeGen
The frontend AST is converted to the custom `mt` MLIR dialect (`MLIRGen`).
- Constructs MLIR nodes.
- Preserves high-level semantics (block pointers, tensor arithmetic).

## 4. High-level Optimizer (M10)
Passes run on the `mt` dialect:
- Semantic dead code elimination.
- Constant folding.
- Mathematical canonicalization.

## 5. GPU Lowering (M6-M7)
Translates the `mt` dialect into lower-level MLIR dialects:
- `mt.program_id` -> `gpu.block_id`
- `mt.load` -> Iterative `memref.load` and `scf.for` (for tiled), or `vector.transfer_read` (for shared+vectorized).
- Lowers into `arith`, `scf`, `math`, `nvvm`, and `llvm` dialects using MLIR's dialect conversion framework.

## 6. Runtime (M8)
The resulting PTX string is submitted to the GPU via the CUDA Driver API (`cuModuleLoadDataEx`, `cuLaunchKernel`). The `minitriton` runtime handles seamless memory allocations and device synchronization.
