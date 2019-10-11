#!/bin/bash

[[ -z "${LLVM_BUILD}" ]] && { echo "Please set LLVM_BUILD environment variable."; }
[[ -z "${LLVM_SRC}" ]] && { echo "Please set LLVM_SRC environment variable."; }

#cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/home/kbarton/Workspaces/LLVMDev2019/WritingLoopOptTutorial/llvm/install -DLLVM_ENABLE_ASSERTIONS=On -DLLVM_ENABLE_PROJECTS=clang;llvm;compiler-rt;clang-tools-extra -DCMAKE_C_COMPILER=/opt/at12.0/bin/gcc -DCMAKE_CXX_COMPILER=/opt/at12.0/bin/g++ -DCMAKE_C_COMPILER_EXTERNAL_TOOLCHAIN:PATH=/home/llvm/gcc/7.3.0/ -DCMAKE_CXX_COMPILER_EXTERNAL_TOOLCHAIN:PATH=/home/llvm/gcc/7.3.0/ -DLLVM_BINUTILS_INCDIR=/usr/include '"-DGCC_INSTALL_PREFIX=/home/llvm/gcc/7.3.0/"' -DLLVM_CCACHE_BUILD=ON '-DLLVM_LIT_ARGS=--threads=20 -v -s' /home/kbarton/Workspaces/LLVMDev2019/WritingLoopOptTutorial/llvm/src/llvm

# This is a minimal build for the purposes of the tutorial.
# It only builds the LLVM project.
# It only builds the X86 and PowerPC backends.
# It uses CCACHE for improved build times.
# It builds shared libraries, which reduces the link time when rebuilbing.

# Configure
cd ${LLVM_BUILD}
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/home/kbarton/Workspaces/LLVMDev2019/WritingLoopOptTutorial/llvm/install -DLLVM_ENABLE_ASSERTIONS=On -DLLVM_ENABLE_PROJECTS="llvm" -DLLVM_TARGETS_TO_BUILD="X86;PowerPC" -DCMAKE_C_COMPILER=/opt/at12.0/bin/gcc -DCMAKE_CXX_COMPILER=/opt/at12.0/bin/g++ -DCMAKE_C_COMPILER_EXTERNAL_TOOLCHAIN:PATH=/home/llvm/gcc/7.3.0/ -DCMAKE_CXX_COMPILER_EXTERNAL_TOOLCHAIN:PATH=/home/llvm/gcc/7.3.0/ -DLLVM_BINUTILS_INCDIR=/usr/include '"-DGCC_INSTALL_PREFIX=/home/llvm/gcc/7.3.0/"' -DLLVM_CCACHE_BUILD=ON -DBUILD_SHARED_LIBS=ON '-DLLVM_LIT_ARGS=--threads=20 -v -s' /home/kbarton/Workspaces/LLVMDev2019/WritingLoopOptTutorial/llvm/src/llvm

# Build
cd ${LLVM_BUILD}
ninja -j20
