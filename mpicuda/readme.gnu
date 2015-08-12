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
cd /apps/escha/mchquickstart.git/mpicuda/CRAY/
nvcc -arch=sm_37 -c ../src/mpicu.cu 
mpicc -c ../src/mpic.c
mpicxx  mpic.o mpicu.o      # link with cc => undefined reference to __gxx_personality_v0



## Run ----
export OMP_NUM_THREADS=1
srun --exclusive -n1 a.out 12
	# 0 6 11
	# 0 606 1111 
	# 
	# Process 0 on keschcn-0009 out of 1 Device 0 (Tesla K80)
	# Device 0: "Tesla K80"
	#   CUDA Driver Version / Runtime Version     6.5 / 6.5
	#   CUDA Capability Major/Minor version number:    3.7
	#   NVRM version: NVIDIA UNIX x86_64 Kernel Module  340.87  Thu Mar 19 23:39:02 PDT 2015
