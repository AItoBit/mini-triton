//===- CUDARuntime.h - CUDA Driver API wrapper ----------------------------===//
//
// Simple wrapper over CUDA Driver API (cuda.h) to load PTX strings and execute
// them directly without needing nvcc.
//
//===----------------------------------------------------------------------===//

#pragma once

#include <string>
#include <vector>

namespace minitriton {
namespace rt {

struct CUDABuffer {
    void* d_ptr;
    size_t size;
};

class CUDARuntime {
public:
    CUDARuntime();
    ~CUDARuntime();

    /// Initialize CUDA Driver API and create a context on the first device.
    bool initialize();

    /// Load a PTX string and extract the kernel function by name.
    bool loadKernel(const std::string& ptx, const std::string& kernelName);

    /// Allocate device memory.
    CUDABuffer allocate(size_t sizeBytes);

    /// Copy host to device.
    void copyHtoD(CUDABuffer& dst, const void* src);

    /// Copy device to host.
    void copyDtoH(void* dst, const CUDABuffer& src);

    /// Free device memory.
    void free(CUDABuffer& buf);

    /// Launch the currently loaded kernel.
    void launch(int gridX, int gridY, int gridZ,
                int blockX, int blockY, int blockZ,
                void** args);

    /// Synchronize the device.
    void synchronize();

private:
    void* cuContext_ = nullptr;
    void* cuModule_ = nullptr;
    void* cuFunction_ = nullptr;
    bool initialized_ = false;
};

} // namespace rt
} // namespace minitriton
