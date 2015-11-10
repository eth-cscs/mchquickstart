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
For compilation with GNU compilers for CPU, the recommended programming environment is: 
```
> module load PrgEnv-gnu/15.11
```

For compilation with GNU compilers for GPU, the recommended programming environment is: 
```
> module load PrgEnv-gnu/15.11_cuda_7.0_gdr
```

## CCE programming environment
For compilation with CCE compilers, the recommended programming environment is: 
```
> module load PrgEnv-cray/15.10_cuda_7.0
```
Note that there are only 4 licence available on Kesch/Escha, the cce compiler should be primarily used for compiling OpenACC GPU code

You can find info about PrgEnv modules using:
```
> module help PrgEnv-NAME
```

  
