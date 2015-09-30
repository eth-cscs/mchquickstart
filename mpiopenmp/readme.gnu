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
# module load craype-accel-nvidia35
module list -t
	# Currently Loaded Modulefiles:
	# craype-haswell
	# binutils/2.24
	# GCC/4.8.2-EB
	# cudatoolkit/6.5.14
	# MVAPICH2/2.0.1-GCC-4.8.2-EB
	# gmvapich2/2015b
	# PrgEnv-gnu/2015b

## Compile ----
mkdir -p GNU
cd GNU

### Fortran
mpicc -c ../src/cpuid.c
mpif90 -fopenmp -march=native cpuid.o ../src/hello_world_mpi_openmp.F90 -o hello_world_mpi_openmp_F90.exe
ldd ./hello_world_mpi_openmp_F90.exe | grep "not"

### C
mpicc -c ../src/cpuid.c
mpicc -fopenmp -march=native cpuid.o ../src/hello_world_mpi_openmp.c -o hello_world_mpi_openmp_c.exe
ldd ./hello_world_mpi_openmp_c.exe | grep "not"

### C++
mpicxx -c ../src/cpuid.c
mpicxx -fopenmp -march=native cpuid.o ../src/hello_world_mpi_openmp.cpp -o hello_world_mpi_openmp_cpp.exe
ldd ./hello_world_mpi_openmp_cpp.exe | grep "not"


## Run ----
# man srun
export OMP_NUM_THREADS=2

### Fortran
srun --gres=gpu:1 --exclusive -n3 -c$OMP_NUM_THREADS ./hello_world_mpi_openmp_F90.exe
 Hello world! I am process            1 , thread            0          13
 Hello world! I am process            1 , thread            1          13
 Hello world! I am process            0 , thread            1           1
 Hello world! I am process            2 , thread            0           2
 Hello world! I am process            0 , thread            0           1
 Hello world! I am process            2 , thread            1           2

### C
srun --gres=gpu:1 --exclusive -n3 -c$OMP_NUM_THREADS ./hello_world_mpi_openmp_c.exe
Hello world! From thread 0 of 2 from process 1 of 3 on keschcn-0005 13
Hello world! From thread 1 of 2 from process 1 of 3 on keschcn-0005 13
Hello world! From thread 0 of 2 from process 2 of 3 on keschcn-0005 2
Hello world! From thread 1 of 2 from process 2 of 3 on keschcn-0005 2
Hello world! From thread 0 of 2 from process 0 of 3 on keschcn-0005 0
Hello world! From thread 1 of 2 from process 0 of 3 on keschcn-0005 0

### C++
srun --gres=gpu:1 --exclusive -n3 -c$OMP_NUM_THREADS ./hello_world_mpi_openmp_cpp.exe
mpi task 0/3 openmp thread 0/2 core=1
mpi task 1/3 openmp thread 0/2 core=13
mpi task 1/3 openmp thread 1/2 core=13
mpi task 2/3 openmp thread 0/2 core=2
mpi task 0/3 openmp thread 1/2 core=1
mpi task 2/3 openmp thread 1/2 core=2
