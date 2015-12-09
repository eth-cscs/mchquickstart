!! This file is not up to date, do not use on Kesch !!
#!/bin/bash

# Based on the latest PrgEnv-gnu ----

## Setup ----

module help PrgEnv-gnu/2015b
module show gmvolf/2015b


module purge
module load craype-haswell
module load PrgEnv-gnu/2015b
module load netCDF/4.3.3.1-gmvolf-2015b

module list -t
       # Currently Loaded Modulefiles:
       # craype-haswell
       # binutils/2.24
       # GCC/4.8.2-EB
       # cudatoolkit/6.5.14
       # MVAPICH2/2.0.1-GCC-4.8.2-EB
       # gmvapich2/2015b
       # PrgEnv-gnu/2015b
       # OpenBLAS/0.2.13-GCC-4.8.2-EB-LAPACK-3.5.0
       # FFTW/3.3.4-gmvapich2-2015b
       # ScaLAPACK/2.0.2-gmvapich2-2015b-OpenBLAS-0.2.13-LAPACK-3.5.0
       # gmvolf/2015b
       # zlib/1.2.8-gmvolf-2015b
       # Szip/2.1-gmvolf-2015b
       # HDF5/1.8.15-gmvolf-2015b
       # netCDF/4.3.3.1-gmvolf-2015b

## Compile ----
mkdir -p GNU
cd GNU

mpicc -DNETCDF4 -DPARIO ../src/io.c -o ../bin/netcdf_write.x `pkg-config --libs netcdf` -L/opt/local/slurm/default/lib64/ -lpmi
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
