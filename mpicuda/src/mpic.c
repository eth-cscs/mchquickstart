// --- CSCS (Swiss National Supercomputing Center) ---

#include <stdio.h>
#include <mpi.h>
#include <unistd.h>
#include <stdlib.h>

#ifndef DEVS_PER_NODE
#define DEVS_PER_NODE 16 // Devices per node
#endif


void set_gpu(int);
void get_gpu_info(char *, int);
void get_more_gpu_info(int);
void run_gpu_kernel(int);

int main(int argc, char *argv[])
{
  int rank=0, size=0, namelen;
  char gpu_str[256] = "";
  char processor_name[MPI_MAX_PROCESSOR_NAME];
  MPI_Init (&argc, &argv);
  MPI_Comm_rank (MPI_COMM_WORLD, &rank);  
  MPI_Comm_size (MPI_COMM_WORLD, &size);  
  MPI_Get_processor_name(processor_name, &namelen);
  int dev = rank % DEVS_PER_NODE;
  set_gpu(dev);

  // check any missing argument
  int VECSIZE=0;
  if ( argc != 2 ) {
      printf( "VECTORSIZE argument is missing ! Usage: %s VECTORSIZE, exiting now...\n", argv[0] );
      MPI_Finalize();
      return -1;
  } else {
      VECSIZE = atoi(argv[1]);  // won't work if mpi tasks > 1 or N>12
  }



  // do some work...
   run_gpu_kernel(VECSIZE);




// cudaGetDeviceProperties
  get_gpu_info(gpu_str, dev);
  printf("Process %d on %s out of %d Device %d (%s)\n", rank, processor_name, size, dev, gpu_str); 
  get_more_gpu_info(dev); 
//  MPI_Barrier(MPI_COMM_WORLD);





// /proc/driver/nvidia/version
  if (rank == 0 ) { 
        static const char filename[] = "/proc/driver/nvidia/version";
        FILE *file = fopen ( filename, "r" );
        // warning: ignoring return value of 'fgets', declared with attribute warn_unused_result [-Wunused-result]
        if ( file != NULL ) {
                if ( fgets ( gpu_str, sizeof gpu_str, file ) != NULL ) {
                fputs ( gpu_str, stdout ); 
                }
        }
        fclose(file);
  }



  MPI_Finalize();
  return 0;
}
