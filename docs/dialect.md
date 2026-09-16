# MiniTriton MLIR Dialect

## Namespace: `mt`

The `mt` dialect represents high-level GPU tensor operations before lowering to concrete GPU execution.

## Operations

| Operation | Description |
|-----------|-------------|
| `mt.kernel` | Top-level kernel function |
| `mt.program_id` | Get block/program ID |
| `mt.constant` | Constant value |
| `mt.arange` | Range vector |
| `mt.add` | Addition |
| `mt.sub` | Subtraction |
| `mt.mul` | Multiplication |
| `mt.div` | Division |
| `mt.cmp` | Comparison |
| `mt.load` | Load from global memory |
| `mt.store` | Store to global memory |
| `mt.reduce` | Reduction operation |

## Types

| Type | MLIR Representation |
|------|---------------------|
| `i32` | `i32` (builtin) |
| `f32` | `f32` (builtin) |
| `ptr<f32>` | `!mt.ptr<f32>` |
| `tensor<f32>` | `tensor<?xf32>` or `!mt.tensor<256xf32>` |

## Example

```mlir
mt.kernel @vector_add(%A: !mt.ptr<f32>, %B: !mt.ptr<f32>, %C: !mt.ptr<f32>, %N: i32) {
    %pid = mt.program_id {axis = 0}
    %off = mt.arange {start = 0, end = 256}
    %base = mt.mul %pid, %cst_256
    %idx = mt.add %base, %off
    %mask = mt.cmp lt, %idx, %N
    %a = mt.load %A, %idx, %mask
    %b = mt.load %B, %idx, %mask
    %c = mt.add %a, %b
    mt.store %C, %idx, %c, %mask
}
```
