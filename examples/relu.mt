@kernel
def relu(X: ptr<f32>, Y: ptr<f32>, N: i32):
    pid = program_id(0)
    offset = pid * BLOCK + arange(0, BLOCK)
    mask = offset < N
    x = load(X + offset, mask)
    zero = constant(0.0)
    y = max(x, zero)
    store(Y + offset, y, mask)
