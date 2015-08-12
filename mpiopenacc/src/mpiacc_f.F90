! --- CSCS (Swiss National Supercomputing Center) ---

program saxpy_f90

        use mpi
        implicit none
        integer :: ierr, rank, size, N
        real :: a
        real, dimension(:), allocatable :: b, c
        character(len=9) :: vecsize_in

        call MPI_INIT ( ierr )
        call MPI_COMM_SIZE (MPI_COMM_WORLD, size, ierr)
        call MPI_COMM_RANK (MPI_COMM_WORLD, rank, ierr)

        call GET_COMMAND_ARGUMENT(1, vecsize_in) !, len, status)
        read (vecsize_in ,'(I10)') N ! Convert a string to an integer
        print*, "using MPI with ", size, " PEs, N=", N

        allocate(b(1:N))
        allocate(c(1:N))
        call run(N)
        deallocate(b)
        deallocate(c)
        call MPI_FINALIZE ( ierr )

contains

subroutine run(N)

        integer, intent(in) :: N
        integer :: i

        call set (N)
#ifdef _OPENACC
        print*, "_OPENACC version:", _OPENACC
        call saxpy(N, 1.0)
#endif
        print*, "c[1]=", c(1)
        print*, "c[N/2]=", c(N/2)
        print*, "c[N-1]=", c(N-1)
        !call MPI_Barrier(MPI_COMM_WORLD, ierr)

end subroutine run

subroutine set(N)

        integer, intent(IN) :: N
        integer :: i
!$acc parallel loop
        do i= 1, N
           b(i)=i*1.0
           c(i)=i*100.0
        enddo
!$acc end parallel loop

end subroutine set

subroutine saxpy(n, a)

        integer, intent(in) :: n
        real, intent(in) :: a
        integer :: i

!$acc parallel loop copyin(b) copy(c)
        do i = 1, n
                c(i) = c(i) + a*b(i)
        enddo
!$acc end parallel loop

end subroutine saxpy

end program saxpy_f90
