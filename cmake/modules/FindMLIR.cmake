#===- CMakeLists.txt - CMake logic for finding MLIR -------------------------===#
#
# Helper module to find and depend on MLIR.
#
#===----------------------------------------------------------------------===#

find_package(MLIR REQUIRED CONFIG)

message(STATUS "Found MLIR: \${MLIR_DIR}")

list(APPEND CMAKE_MODULE_PATH "\${MLIR_CMAKE_DIR}")
list(APPEND CMAKE_MODULE_PATH "\${LLVM_CMAKE_DIR}")

include(TableGen)
include(AddLLVM)
include(AddMLIR)

# Setup include directories
include_directories(\${LLVM_INCLUDE_DIRS})
include_directories(\${MLIR_INCLUDE_DIRS})

# Library definitions
set(MLIR_LIBS
    MLIRIR
    MLIRBuiltinToLLVMIRTranslation
    MLIRExecutionEngine
    MLIRTargetLLVMIRExport
    MLIRToLLVMIRTranslationRegistration
    MLIRTransforms
    MLIRArithToLLVM
    MLIRVectorToLLVM
    MLIRGPUToNVVM
    MLIRNVVMToLLVM
    MLIRLinalgToLLVM
    MLIRMathToLLVM
    MLIRSCFToControlFlow
    MLIRControlFlowToLLVM
)
