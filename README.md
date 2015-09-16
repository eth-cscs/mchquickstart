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
```module load PrgEnv-gnu/2015b```
This will set GCC/4.8.2 as the default compiler. The gcc/4.8.2 modulefile installed by Cray cannot assemble haswell instructions (thus -march=native is not working). Cray is aware of the problem and is working on a permanent solution. [GPP]

## CCE programming environment
For compilation with GNU compilers, the recommended programming environment is: 
```module load PrgEnv-cray/1.0.0```
