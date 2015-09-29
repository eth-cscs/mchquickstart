program hello_world_mpi_openmp

        !use mpi
        use omp_lib
        implicit none
        include 'mpif.h'

        integer :: rank, nproc, ierr, i, id
        integer, dimension(MPI_STATUS_SIZE) :: status

        INTERFACE
          function mycpuid() BIND(C)
            USE, INTRINSIC :: ISO_C_BINDING
            IMPLICIT NONE
            INTEGER(C_INT) :: mycpuid
          end function mycpuid
        END INTERFACE

        call MPI_INIT(ierr)
        call MPI_COMM_RANK(MPI_COMM_WORLD, rank, ierr)
        call MPI_COMM_SIZE(MPI_COMM_WORLD, nproc, ierr)
        
!$OMP PARALLEL PRIVATE(id)
        id = OMP_GET_THREAD_NUM()
        print *, 'Hello world! I am process ', rank, ', thread ', id, mycpuid()
!$OMP END PARALLEL

        call MPI_FINALIZE(ierr)

end program hello_world_mpi_openmp

