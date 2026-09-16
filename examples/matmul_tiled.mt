@kernel
def matmul_tiled(A: ptr<f32>, B: ptr<f32>, C: ptr<f32>, M: i32, N: i32, K: i32):
    pid_m = program_id(0)
    pid_n = program_id(1)
    
    offs_am = (pid_m * BLOCK_M + arange(0, BLOCK_M)) % M
    offs_bn = (pid_n * BLOCK_N + arange(0, BLOCK_N)) % N
    offs_k = arange(0, BLOCK_K)
    
    a_ptrs = A + (offs_am * K)
    b_ptrs = B + offs_bn
    
    acc = constant(0.0)
    
    # Pseudo-code for a loop over K dimension in chunks of BLOCK_K.
    # The actual MiniTriton compiler lowers this via SCF.for loop in MLIR.
    
    # for k in range(0, K, BLOCK_K):
    #     a = load(a_ptrs + (k + offs_k))
    #     b = load(b_ptrs + ((k + offs_k) * N))
    #     acc = acc + dot(a, b)
    
    # store(C + (offs_am * N) + offs_bn, acc)
