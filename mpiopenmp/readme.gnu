#!/bin/bash

## Setup ----
module use $APPS/easybuild/modules/all
module load gmvolf/2015a
module help gmvolf/2015a
	# ----------- Module Specific Help for 'gmvolf/2015a' ---------------
	#  GNU Compiler Collection (GCC) based compiler toolchain, including
	#  MVAPICH2 for MPI support, OpenBLAS (BLAS and LAPACK support), FFTW and ScaLAPACK.
module load craype-accel-nvidia35
module list -t
	# Currently Loaded Modulefiles:
	# craype-haswell
	# GCC/4.8.2
	# cudatoolkit/6.5.14
	# MVAPICH2/2.0.1_gnu48
	# OpenBLAS/0.2.13-GCC-4.8.2-LAPACK-3.5.0
	# gmvapich2/2015a
	# FFTW/3.3.4-gmvapich2-2015a
	# ScaLAPACK/2.0.2-gmvapich2-2015a-OpenBLAS-0.2.13-LAPACK-3.5.0
	# gmvolf/2015a

## Compile ----
cd /apps/escha/mchquickstart.git/mpiopenmp/GNU/
mpif90 -fopenmp ../src/hello_world_mpi_openmp.f90     # Fortran
mpicc -fopenmp ../src/hello_world_mpi_openmp.c        # C
mpicxx -fopenmp ../src/hello_world_mpi_openmp.cpp     # C++

## Run ----
export OMP_NUM_THREADS=2
man srun
srun --exclusive -n3 -c$OMP_NUM_THREADS ./a.out 
	# Hello world! From thread 0 of 2 from process 0 of 3 on keschcn-0009
	# Hello world! From thread 1 of 2 from process 0 of 3 on keschcn-0009
	# Hello world! From thread 1 of 2 from process 1 of 3 on keschcn-0009
	# Hello world! From thread 0 of 2 from process 1 of 3 on keschcn-0009
	# Hello world! From thread 1 of 2 from process 2 of 3 on keschcn-0009
	# Hello world! From thread 0 of 2 from process 2 of 3 on keschcn-0009
