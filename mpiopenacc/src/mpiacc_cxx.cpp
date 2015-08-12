// --- CSCS (Swiss National Supercomputing Center) ---

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>

// #include <omp.h>
#include <mpi.h>

void set(double *a, double *b, double *c, int N);
void saxpy( int n, double a, double *x, double *y);
void run(int rank, int N);

void run(int rank, int N)
{
    // initialize application
    MPI_Barrier(MPI_COMM_WORLD);
    double *a = reinterpret_cast<double*>(malloc(N*sizeof(double)));
    double *b = reinterpret_cast<double*>(malloc(N*sizeof(double)));
    double *c = reinterpret_cast<double*>(malloc(N*sizeof(double)));

    set(a,b,c,N);
#ifdef _OPENACC
    std::cout << "_OPENACC version:" << _OPENACC << std::endl;    
    saxpy(N, 1.0, b, c);
#endif

    std::cout << "c[0]=" << c[0] << std::endl;
    std::cout << "c[1]=" << c[1] << std::endl;
    std::cout << "c[N/2]=" << c[N/2] << std::endl;
    std::cout << "c[N-1]=" << c[N-1] << std::endl;

    free(a);
    free(b);
    free(c);
}
 


int main(int argc, char **argv){
    // init mpi
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    //const int N = 4600;
    int N = atoi(argv[1]);  //argv[0] is the program name
    if(!rank) std::cout << "using MPI with " << size << " PEs, N=" << N << std::endl;
    run(rank, N);

    // finalize MPI
    MPI_Finalize();

   return 0;
}

void set(double *a, double *b, double *c, int N) {
    // #pragma omp parallel for
    for(int i=0; i<N; i++){
        a[i] = 2.;
        b[i] = i*1.0;
        c[i] = i*100.0;
    }
}

void saxpy( int n, double a, double *x, double *y){
    int i;      // do not use gdb/76, use cuda-gdb
    #pragma acc parallel loop pcopyin(x[0:n]) pcopy(y[0:n])
    for( i = 0; i < n; ++i )
        y[i] += a*x[i];
}
