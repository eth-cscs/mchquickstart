#!/bin/bash

# Based on the latest PrgEnv-gnu ----


## Setup ----
module avail PrgEnv-gnu
module help  PrgEnv-gnu/15.11

module purge
module load craype-haswell
module load PrgEnv-gnu/15.11

module list -t
  # Currently Loaded Modulefiles:
  # craype-haswell
  # binutils/2.25
  # GCC/4.9.3-binutils-2.25
  # cudatoolkit/7.0.28
  # MVAPICH2/2.2a-GCC-4.9.3-binutils-2.25
  # gmvapich2/15.11
  # PrgEnv-gnu/15.11

            
## Compile ----
mkdir -p GNU
cd GNU
ls -al

nvcc -arch=sm_37 -c ../src/mpicu.cu 
mpicc -c ../src/mpic.c
mpicc  mpic.o mpicu.o  -o mpi_cuda_test.exe `pkg-config --libs cudart`

ls -al 
ldd mpi_cuda_test.exe
ldd mpi_cuda_test.exe | grep -i "not found"
ldd mpi_cuda_test.exe | grep -i "cuda" 
ldd mpi_cuda_test.exe | grep -i "cuda" | egrep -v 7\.0


## Run ----
salloc --exclusive --gres=gpu:12 --ntasks=12 --time=00:05:00 --job-name="mpicuda-gnu" 

export OMP_NUM_THREADS=1
srun   --exclusive --gres=gpu:1  --ntasks=1 mpi_cuda_test.exe 12
  #0 6 11
  #0 606 1111
  #Process 0 on keschcn-0003 out of 1 Device 0 (Tesla K80)
  #
  #Device 0: "Tesla K80"
  #  CUDA Driver Version / Runtime Version     7.0 / 7.0
  #  CUDA Capability Major/Minor version number:    3.7
  #NVRM version: NVIDIA UNIX x86_64 Kernel Module  346.82  Wed Jun 17 10:37:46 PDT 2015
