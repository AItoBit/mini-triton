@kernel
def matmul(A: ptr<f32>, B: ptr<f32>, C: ptr<f32>, M: i32, N: i32, K: i32):
    row = program_id(0)
    col = program_id(1)
    acc = constant(0.0)
    k = arange(0, BLOCK_K)
    a = load(A + row * K + k)
    b = load(B + k * N + col)
    acc = acc + reduce_sum(a * b)
    store(C + row * N + col, acc)
