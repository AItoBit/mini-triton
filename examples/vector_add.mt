@kernel
def vector_add(A: ptr<f32>, B: ptr<f32>, C: ptr<f32>, N: i32):
    pid = program_id(0)
    offset = pid * BLOCK + arange(0, BLOCK)
    mask = offset < N
    a = load(A + offset, mask)
    b = load(B + offset, mask)
    store(C + offset, a + b, mask)
