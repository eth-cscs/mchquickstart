#!/bin/bash

# Based on the latest PrgEnv-gnu ----

## Setup ----
# module use $APPS/easybuild/modules/all
module help PrgEnv-gnu/2015b
module help gmvolf/2015b
        # ----------- Module Specific Help for 'gmvolf/2015b' -----------
        #  GNU Compiler Collection (GCC) based compiler toolchain, including
        #  MVAPICH2 for MPI support, OpenBLAS (BLAS and LAPACK support), FFTW and ScaLAPACK.

module purge
module load craype-haswell
module load PrgEnv-gnu/2015b
module load craype-accel-nvidia35

module list -t
	# Currently Loaded Modulefiles:
	# craype-haswell
	# binutils/2.24
	# GCC/4.8.2-EB
	# cudatoolkit/6.5.14
	# MVAPICH2/2.0.1-GCC-4.8.2-EB
	# OpenBLAS/0.2.13-GCC-4.8.2-EB-LAPACK-3.5.0
	# gmvapich2/2015b
	# FFTW/3.3.4-gmvapich2-2015b
	# ScaLAPACK/2.0.2-gmvapich2-2015b-OpenBLAS-0.2.13-LAPACK-3.5.0
	# gmvolf/2015b
	# PrgEnv-gnu/2015b
	# cray-libsci_acc/3.1.2
	# craype-accel-nvidia35
        
## Compile ----
mkdir -p GNU
cd GNU
nvcc -arch=sm_37 -c ../src/mpicu.cu 
mpicc -c ../src/mpic.c
mpicc  mpic.o mpicu.o  -o mpi_cuda_test.exe -L$CUDATOOLKIT_HOME/lib64 -lcudart  -lstdc++
ldd mpi_cuda_test.exe |grep "not found"
# Remark: linking with mpicc will fail with the following error message: undefined reference to __gxx_personality_v0
#         workaround is to link with mpicxx instead of mpicc or add -lstdc++



## Run ----
export OMP_NUM_THREADS=1
srun --gres=gpu:1 --exclusive -n1 mpi_cuda_test.exe 12
	# 0 6 11
	# 0 606 1111
	# Process 0 on keschcn-0001 out of 1 Device 0 (Tesla K80)
	# 
	# Device 0: "Tesla K80"
	#   CUDA Driver Version / Runtime Version     6.5 / 6.5
	#   CUDA Capability Major/Minor version number:    3.7
	# NVRM version: NVIDIA UNIX x86_64 Kernel Module  340.87  Thu Mar 19 23:39:02 PDT 2015


## Todo
## mvapich2gdr_gnu/2.1rc2
