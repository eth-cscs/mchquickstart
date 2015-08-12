/* requires console i/o on all mpi processes, so might fail, twr */
#include <iostream>
#include <mpi.h>   
#include <omp.h>

int main(int argc, char *argv[]) {
  int rank, size, th_id, nthreads;

  MPI::Init(argc, argv);

  rank = MPI::COMM_WORLD.Get_rank();
  size = MPI::COMM_WORLD.Get_size();
#pragma omp parallel private(th_id) shared(nthreads)
  {
  th_id = omp_get_thread_num();
  nthreads = omp_get_num_threads();
#pragma omp critical
  std::cout << "mpi task " << rank << "/" << size << " openmp thread " << th_id << "/" << nthreads << std::endl;
  }
  MPI::Finalize();

  return 0;
} /* end func main */
