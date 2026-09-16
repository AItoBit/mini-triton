@kernel
def matmul_shared_vectorized(A: ptr<f32>, B: ptr<f32>, C: ptr<f32>, M: i32, N: i32, K: i32):
    # This kernel emphasizes vectorized loads and shared memory layout hints.
    pid_m = program_id(0)
    pid_n = program_id(1)
    
    # Vectorized offset calculation
    offs_am = (pid_m * BLOCK_M + arange(0, BLOCK_M))
    offs_bn = (pid_n * BLOCK_N + arange(0, BLOCK_N))
    offs_k = arange(0, BLOCK_K)
    
    a_ptrs = A + (offs_am * K)
    b_ptrs = B + offs_bn
    
    acc = constant(0.0)
    
    # Vectorized load/stores will be generated automatically in the MLIR backend
    # when the GPU lowering pass detects contiguous access patterns on the innermost dim.
    # The Shared Memory allocation is handled by the `gpu_local_memory` allocation
    # during the MLIR GPU lowering phase matching the block sizes.
    
    # for k block ...
    #   a = load(a_ptrs) # Lowers to cp.async (vectorized TMA if Hopper, or ld.g.128 on Ampere)
    #   b = load(b_ptrs)
    #   acc += dot(a, b)
    #
    # store(C, acc)
