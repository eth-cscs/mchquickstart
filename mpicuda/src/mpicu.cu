// --- CSCS (Swiss National Supercomputing Center) ---

#include <stdio.h>

extern "C"
void set_gpu(int dev)
{
  cudaSetDevice(dev);
}

extern "C"
void get_gpu_info(char *gpu_string, int dev)
{
  struct cudaDeviceProp dprop;
  cudaGetDeviceProperties(&dprop, dev);
  strcpy(gpu_string,dprop.name);
}





extern "C"
void get_more_gpu_info(int dev)
{
  int driverVersion = 0, runtimeVersion = 0;
  struct cudaDeviceProp deviceProp;
  cudaGetDeviceProperties(&deviceProp, dev);
  printf("\nDevice %d: \"%s\"\n", dev, deviceProp.name);
  cudaDriverGetVersion(&driverVersion);
  cudaRuntimeGetVersion(&runtimeVersion);
  printf("  CUDA Driver Version / Runtime Version     %d.%d / %d.%d\n", driverVersion/1000, (driverVersion%100)/10, runtimeVersion/1000, (runtimeVersion%100)/10);
  printf("  CUDA Capability Major/Minor version number:    %d.%d\n", deviceProp.major, deviceProp.minor);
  
}





// Add two arrays on the device
__global__ void gpu_kernel(int *d_a1, int *d_a2, int *d_a3, int N) {
  int idx = blockIdx.x*blockDim.x + threadIdx.x;
  if (idx < N)
    d_a3[idx] = d_a1[idx] + d_a2[idx];
  d_a1[idx] = idx ; // dummy
}







//#define SIZE 12
extern "C"
void run_gpu_kernel(int SIZE) {

  int i; 
  int a1[SIZE], a2[SIZE], a3[SIZE];  // Host arrays
  int *d_a1, *d_a2, *d_a3;           // Device arrays
  
  // Initalize the host input arrays
  for(i = 0; i < SIZE; i++) {
    a1[i] = i;
    a2[i] = 100*i;
  }
  
  // Allocate the device arrays and copy data over to the device
  cudaMalloc((void**) &d_a1, sizeof(int)*SIZE);
  cudaMalloc((void**) &d_a2, sizeof(int)*SIZE);
  cudaMalloc((void**) &d_a3, sizeof(int)*SIZE);  
  cudaMemcpy(d_a1, a1, sizeof(int)*SIZE, cudaMemcpyHostToDevice);
  cudaMemcpy(d_a2, a2, sizeof(int)*SIZE, cudaMemcpyHostToDevice);

  // Zero out results
  cudaMemset(d_a3, 0, sizeof(int)*SIZE);
  
  gpu_kernel<<<3, 4>>>(d_a1, d_a2, d_a3, SIZE);
  
  cudaMemcpy(a3, d_a3, sizeof(int)*SIZE, cudaMemcpyDeviceToHost);
  
  printf("%d %d %d\n", 0, SIZE/2, SIZE-1);
  printf("%d ", a3[0]);
  printf("%d ", a3[SIZE/2]);
  printf("%d ", a3[SIZE-1]);
  printf("\n");

  cudaFree(d_a1);
  cudaFree(d_a2);
  cudaFree(d_a3);
}
