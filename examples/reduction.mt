@kernel
def sum_reduce(X: ptr<f32>, Out: ptr<f32>, N: i32):
    pid = program_id(0)
    offset = pid * BLOCK + arange(0, BLOCK)
    mask = offset < N
    x = load(X + offset, mask)
    total = reduce_sum(x)
    store(Out + pid, total)
