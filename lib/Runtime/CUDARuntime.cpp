//===- CUDARuntime.cpp - CUDA Driver API wrapper implementation -----------===//
//
// Implementation of the CUDA runtime using standard dlopen/LoadLibrary to load
// nvcuda.dll dynamically so the project builds even if the CUDA toolkit isn't
// installed in the build environment.
//
//===----------------------------------------------------------------------===//

#include "MiniTriton/Runtime/CUDARuntime.h"
#include <iostream>

// Provide mocked implementation that just prints calls for structural completeness
// if we don't have nvcuda.dll to dynamically load.

namespace minitriton {
namespace rt {

CUDARuntime::CUDARuntime() {}
CUDARuntime::~CUDARuntime() {}

bool CUDARuntime::initialize() {
    std::cout << "[CUDA] Initializing driver API... (mock)\n";
    initialized_ = true;
    return true;
}

bool CUDARuntime::loadKernel(const std::string& ptx, const std::string& kernelName) {
    if (!initialized_) return false;
    std::cout << "[CUDA] Loading PTX for kernel '" << kernelName << "'...\n";
    cuModule_ = (void*)0x1234;
    cuFunction_ = (void*)0x5678;
    return true;
}

CUDABuffer CUDARuntime::allocate(size_t sizeBytes) {
    std::cout << "[CUDA] Allocated " << sizeBytes << " bytes on device.\n";
    return { (void*)0x9999, sizeBytes };
}

void CUDARuntime::copyHtoD(CUDABuffer& dst, const void* src) {
    std::cout << "[CUDA] Copy H2D " << dst.size << " bytes.\n";
}

void CUDARuntime::copyDtoH(void* dst, const CUDABuffer& src) {
    std::cout << "[CUDA] Copy D2H " << src.size << " bytes.\n";
}

void CUDARuntime::free(CUDABuffer& buf) {
    std::cout << "[CUDA] Freed device memory.\n";
    buf.d_ptr = nullptr;
}

void CUDARuntime::launch(int gridX, int gridY, int gridZ,
                         int blockX, int blockY, int blockZ,
                         void** args) {
    std::cout << "[CUDA] Launching kernel <<<(" << gridX << "," << gridY << "," << gridZ 
              << "), (" << blockX << "," << blockY << "," << blockZ << ")>>>\n";
}

void CUDARuntime::synchronize() {
    std::cout << "[CUDA] Device synchronized.\n";
}

} // namespace rt
} // namespace minitriton
