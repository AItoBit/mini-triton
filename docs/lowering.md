# MiniTriton Lowering Pipeline

## Stages

```
mt dialect → arith + scf + gpu + nvvm → llvm dialect → LLVM IR → PTX
```

### Stage 1: mt → Standard Dialects

| mt Operation | Lowered To |
|--------------|------------|
| `mt.program_id` | `gpu.block_id` / NVVM intrinsic |
| `mt.add/sub/mul/div` | `arith.addf`, `arith.mulf`, etc. |
| `mt.cmp` | `arith.cmpf` / `arith.cmpi` |
| `mt.load` | `memref.load` with masking logic |
| `mt.store` | `memref.store` with masking logic |
| `mt.arange` | `scf.for` + `vector.broadcast` |
| `mt.reduce` | `vector.reduction` |

### Stage 2: Standard → LLVM

Uses MLIR's built-in conversion passes:
- `convert-gpu-to-nvvm`
- `convert-arith-to-llvm`
- `convert-scf-to-cf`
- `convert-func-to-llvm`

### Stage 3: LLVM IR → PTX

Uses LLVM's NVPTX backend to emit PTX assembly.
