# MiniTriton — Architecture

## Compiler Pipeline

```
┌───────────────────────────────┐
│          Frontend             │
│ Lexer / Parser / AST / Types  │
└──────────────┬────────────────┘
               ↓
┌───────────────────────────────┐
│      MiniTriton Dialect       │
│ High-level GPU/Tensor IR      │
└──────────────┬────────────────┘
               ↓
┌───────────────────────────────┐
│         Optimizer             │
│ Canonicalization              │
│ Constant folding              │
│ DCE / Fusion / Tiling         │
└──────────────┬────────────────┘
               ↓
┌───────────────────────────────┐
│        GPU Lowering           │
│ SCF / Vector / GPU / NVVM     │
└──────────────┬────────────────┘
               ↓
┌───────────────────────────────┐
│        LLVM / NVPTX           │
│ LLVM IR → PTX                 │
└──────────────┬────────────────┘
               ↓
┌───────────────────────────────┐
│         CUDA Runtime          │
│ load kernel / launch kernel   │
└──────────────┘
```

## Design Principles

1. **Educational clarity over performance** — every layer should be understandable
2. **Progressive lowering** — each transformation step is explicit and inspectable
3. **Minimal scope** — support only what's needed for the target kernels
4. **MLIR-native** — leverage existing MLIR dialects (arith, scf, gpu, nvvm) for lowering
