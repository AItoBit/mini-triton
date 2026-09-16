# MiniTriton Language Reference

## Syntax

MiniTriton uses Python-like indentation-based syntax.

### Kernel Definition

```python
@kernel
def kernel_name(param1: type1, param2: type2, ...):
    body
```

### Types

| Type | Description |
|------|-------------|
| `i32` | 32-bit integer |
| `f32` | 32-bit float |
| `ptr<T>` | Pointer to type T |

### Builtin Functions

| Function | Signature | Description |
|----------|-----------|-------------|
| `program_id(axis)` | `(i32) → i32` | Returns block/program ID for given axis |
| `arange(start, end)` | `(i32, i32) → tensor<i32>` | Creates a range vector |
| `load(ptr, mask?)` | `(ptr<T>, tensor<i1>?) → tensor<T>` | Loads from global memory |
| `store(ptr, val, mask?)` | `(ptr<T>, tensor<T>, tensor<i1>?) → void` | Stores to global memory |
| `reduce_sum(x)` | `(tensor<T>) → T` | Reduces tensor by summation |
| `constant(val)` | `(literal) → T` | Creates a constant |
| `max(a, b)` | `(T, T) → T` | Element-wise maximum |

### Operators

| Operator | Types | Description |
|----------|-------|-------------|
| `+` `-` `*` `/` | arithmetic | Standard arithmetic |
| `<` `>` `<=` `>=` `==` `!=` | comparison | Returns mask/boolean |

### Constants

| Name | Description |
|------|-------------|
| `BLOCK` | Block size (configurable at compile time) |
| `BLOCK_K` | K-dimension block size (for matmul) |
