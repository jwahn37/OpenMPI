#include "mpi.h"
#include <stdio.h>

int main ( int argc, char *argv[ ] )
{
    int numtasks, rank, next, prev, buf[2], tag1 = 1, tag2 = 2 ;
    MPI_Request reqs[4] ; 
    MPI_Status stats[4] ;
    int i;

    MPI_Init ( &argc, &argv ) ;
    MPI_Comm_size ( MPI_COMM_WORLD, & numtasks ) ; 
    MPI_Comm_rank ( MPI_COMM_WORLD, &rank ) ;

    prev = rank-1;
    next = rank+1;

    if ( rank == 0 )
        prev = numtasks-1;
    if(rank==(numtasks-1))
        next = 0;


    MPI_Irecv ( &buf[0] , 1 , MPI_INT , prev , tag1 , MPI_COMM_WORLD , &reqs[0] ) ; 
    MPI_Irecv ( &buf[1] , 1 , MPI_INT , next , tag2 , MPI_COMM_WORLD , &reqs[1] ) ;
    MPI_Isend ( &rank , 1 , MPI_INT , prev , tag2 , MPI_COMM_WORLD , &reqs[2] ) ; 
    MPI_Isend ( &rank , 1 , MPI_INT , next , tag1 , MPI_COMM_WORLD , &reqs[3] ) ;
   // MPI_Waitall ( 4 , reqs , stats ) ; 
    for(i=0; i<numtasks; i++)
    {
        if(i != rank)
            MPI_Wait(&reqs[i], &stats[i]);
    }

    MPI_Finalize ( ) ;
    printf("rank(%d), %d %d\n", rank, buf[0], buf[1]);
}
