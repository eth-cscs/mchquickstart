# mchquickstart
Introduction for new MCH users

## Getting started
Download this user guide with:

```
> git clone https://github.com/eth-cscs/mchquickstart.git    mchquickstart.git
> cd mchquickstart.git/
```

and start by reading the readme files.

## GNU programming environment
For compilation with GNU compilers, the recommended programming environment is: 
```
> module load PrgEnv-gnu/2015b
```
This will set GCC/4.8.2-EB as the default compiler. 

The gcc/4.8.2 modulefile installed by Cray cannot assemble haswell instructions (thus -march=native is not working). Therefore the recommended GNU compiler currently is GCC/4.8.2-EB (loaded by PrgEnv-gnu/2015b).

## CCE programming environment
For compilation with CCE compilers, the recommended programming environment is: 
```
> module load PrgEnv-cray/1.0.0
```
## EasyBuild Toolchains / PrgEnv

You can find here a short description of the existing PrgEnv / toolchains:
  * Recommended versions:
    * PrgEnv-gnu/2015b: Default GCC + MVAPICH + CUDA
      * GCC/4.8.2-EB, MVAPICH2/2.0.1-GCC-4.8.2-EB
    * gmvapich2/2015b 
      * same modules as PrgEnv-gnu/2015b (used by EasyBuild)
    * gmvolf/2015b
      * PrgEnv-gnu/2015b + OpenBLAS + FFTW + ScaLAPACK

  * Additional versions (under test for Cosmo)
    * PrgEnv-gnu/2015a-gdr-2.1
      * GCC/4.8.2-EB + mvapich2gdr_gnu/2.1 + CUDA
    * PrgEnv-gnu/2015b-gdr: Uses mvapich2gdr installed on hamidouc's home (will be removed)
      * GCC/4.8.2-EB + mvapich2gdr_gnu/2.1rc2 + CUDA
      
  * Deprecated versions (will be removed)
    * gmvapich2-2015a: Uses unsupported gcc/4.8.2
    * gmvolf-2015a:  Uses unsupported gcc/4.8.2
  
