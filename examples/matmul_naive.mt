@kernel
def matmul_naive(A: ptr<f32>, B: ptr<f32>, C: ptr<f32>, M: i32, N: i32, K: i32):
    # Each program instance computes a BLOCK_M x BLOCK_N tile of C
    pid_m = program_id(0)
    pid_n = program_id(1)
    
    # Calculate row and column offsets
    offs_m = pid_m * BLOCK_M + arange(0, BLOCK_M)
    offs_n = pid_n * BLOCK_N + arange(0, BLOCK_N)
    
    # Initialize accumulator
    acc = constant(0.0)

    # Loop over K dimension
    # (Since we don't have for-loops in the frontend yet, this is a conceptual 
    # unrolled block or relies on a higher-level dot instruction in the MLIR.
    # In Triton, this is typically done via a loop, but we can simulate the 
    # dot product semantics).
    
    # Example using a simplified dot operation for the naive approach 
    # assuming BLOCK_K == K for this basic example.
    offs_k = arange(0, BLOCK_K)
    
    a_ptrs = A + (offs_m * K) + offs_k
    b_ptrs = B + (offs_k * N) + offs_n
    
    a = load(a_ptrs)
    b = load(b_ptrs)
    
    acc = acc + dot(a, b)
    
    c_ptrs = C + (offs_m * N) + offs_n
    store(c_ptrs, acc)
