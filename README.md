# MiniTriton — GPU Kernel Compiler

A small educational language and compiler for writing high-level tensor operations and compiling them to NVIDIA GPU kernels.

MiniTriton reproduces [Triton](https://triton-lang.org/)'s idea at an educational scale: a Python-like DSL for GPU programming that compiles through MLIR to PTX.

## Pipeline

```
program.mt → Lexer → Parser → AST → Type Checker → MLIR (mt dialect) → GPU Lowering → LLVM IR → PTX → CUDA Runtime → GPU
```

## Quick Start

### Build

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

### Usage

```bash
# Dump token stream
./minitriton dump-tokens ../examples/vector_add.mt

# Dump AST
./minitriton dump-ast ../examples/vector_add.mt

# Compile (future: produces .mlir, .ll, .ptx)
./minitriton compile ../examples/vector_add.mt

# Compile and run on GPU (future)
./minitriton run ../examples/vector_add.mt
```

## Example

```python
@kernel
def vector_add(A: ptr<f32>, B: ptr<f32>, C: ptr<f32>, N: i32):
    pid = program_id(0)
    offset = pid * BLOCK + arange(0, BLOCK)
    mask = offset < N
    a = load(A + offset, mask)
    b = load(B + offset, mask)
    store(C + offset, a + b, mask)
```

## Project Structure

```
MiniTriton/
├── include/MiniTriton/   # Headers (AST, Frontend, Semantic)
├── lib/                  # Implementation
├── tools/minitriton/     # CLI driver
├── test/                 # Tests
├── examples/             # .mt example programs
├── docs/                 # Documentation
└── benchmarks/           # Performance benchmarks (future)
```

## Milestones

| # | Goal | Status |
|---|------|--------|
| M0 | Project setup | ✅ |
| M1 | Lexer | ✅ |
| M2 | Parser + AST + Type Checker | ✅ |
| M3 | mt MLIR dialect | 🔲 |
| M4 | Source → MLIR | 🔲 |
| M5 | vector_add in MLIR | 🔲 |
| M6-M7 | GPU lowering + PTX | 🔲 |
| M8 | End-to-end vector_add on GPU | 🔲 |
| M9 | ReLU + Reduction kernels | 🔲 |
| M10 | Optimization passes | 🔲 |
| M11-M13 | MatMul (naive → tiled → optimized) | 🔲 |
| M14-M15 | Benchmarks + documentation | 🔲 |

## License

Educational project.
