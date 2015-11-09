#!/bin/bash

# Based on the latest PrgEnv-gnu ----

## Setup ----

module help PrgEnv-gnu/15.11

module purge
module load craype-haswell
module load PrgEnv-gnu/15.11
module load netCDF/4.3.3.1-gmvolf-15.11

module list -t
       # Currently Loaded Modulefiles:
       # craype-haswell
       # binutils/2.25
       # GCC/4.9.3-binutils-2.25
       # cudatoolkit/7.0.28
       # MVAPICH2/2.2a-GCC-4.9.3-binutils-2.25
       # gmvapich2/15.11
       # PrgEnv-gnu/15.11
       # OpenBLAS/0.2.13-GCC-4.9.3-binutils-2.25-LAPACK-3.5.0
       # FFTW/3.3.4-gmvapich2-15.11
       # ScaLAPACK/2.0.2-gmvapich2-15.11-OpenBLAS-0.2.13-LAPACK-3.5.0
       # gmvolf/15.11
       # zlib/.1.2.8-gmvolf-15.11
       # Szip/.2.1-gmvolf-15.11
       # HDF5/1.8.15-gmvolf-15.11
       # netCDF/4.3.3.1-gmvolf-15.11

## Compile ----
mkdir -p GNU
mkdir -p bin
cd GNU

mpicc -DNETCDF4 -DPARIO ../src/io.c -o ../bin/netcdf_write.x `pkg-config --libs netcdf`
ldd ../bin/netcdf_write.x 
ldd ../bin/netcdf_write.x | grep "not found"


## Run ----

salloc  --exclusive --job-name="netcdf-hdf5parallel_write" --nodes=1 --ntasks=24 --ntasks-per-node=24 --time=00:05:00
srun  -n $SLURM_NTASKS ../bin/netcdf_write.x -x 4 -y 6 -f 256.4x6 -b 256 -t 10
       # Testing get_procmem... 869298176.000000 1158582272.000000 289284096.000000
       # written grids of 256,256,256
       # written 10 iterations
       # MPI elapsed time: 29.235340 s
       # Average performance: 0.513078 GB/s


## Cleanup output ----

ls -al 256.4x6.????_p.nc
rm -f  256.4x6.????_p.nc

cd ..
