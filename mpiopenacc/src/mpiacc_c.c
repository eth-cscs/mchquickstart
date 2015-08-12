// --- CSCS (Swiss National Supercomputing Center) ---

#include <stdio.h>
#include <stdlib.h>

// #include <omp.h>
#include <mpi.h>

void saxpy( int n, double a, double *x, double *y);
void run(int rank, const int N);

void run(int rank, const int N)
{
    // MPI_Barrier(MPI_COMM_WORLD);
    double b[N];
    double c[N]; // = reinterpret_cast<double*>(malloc(N*sizeof(double)));

    // #pragma omp parallel for
    for(int i=0; i<N; i++){
        b[i] = i;
        c[i] = i*100.0;
    }
    printf("c[0]=%f\n", c[0]);
    printf("c[N-1]=%f\n", c[N-1]);

#ifdef _OPENACC
    printf("_OPENACC version: %d\n", _OPENACC);    
    saxpy(N, 1.0, b, c);
#endif

    printf("c[0]=%f\n", c[0]);
    printf("c[N-1]=%f\n", c[N-1]);

    // free(b);
    // free(c);
}
 


int main(int argc, char **argv){
    int rank = 0, size = 1;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    //const int N = 4600;
    int N = atoi(argv[1]);  //argv[0] is the program name
    if(!rank) printf("using MPI with %d PEs, N=%d\n", size, N);
    run(rank, N);

    // finalize MPI
    MPI_Finalize();

   return 0;
}

void saxpy( int n, double a, double *x, double *y){
    int i;
    #pragma acc parallel loop pcopyin(x[0:n-1]) pcopyout(y[0:n-1])
    for( i = 0; i < n; ++i )
        y[i] += a*x[i];
}
