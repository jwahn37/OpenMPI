#include "mpi.h"
#include <stdio.h> 
#define SIZE 4

int main ( int argc, char *argv[ ] )
{
    int numtasks, rank, sendcount, recvcount, source;
float sbuf[SIZE][SIZE] = { { 1.0 , 2.0 , 3.0 , 4.0 } , { 5.0 , 6.0 , 7.0 , 8.0 } , { 9.0 , 10.0 , 11.0 , 12.0 } , { 13.0 , 14.0 , 15.0 , 16.0 }} ;
float rbuf[SIZE] ;

MPI_Init ( &argc, &argv ) ;
MPI_Comm_rank ( MPI_COMM_WORLD, &rank ) ; 
MPI_Comm_size ( MPI_COMM_WORLD, &numtasks ) ;

if ( numtasks == SIZE ) {
    source = 1 ; 
    sendcount = SIZE ; 
    recvcount = SIZE ;

    MPI_Scatter ( sbuf , sendcount , MPI_FLOAT , rbuf , recvcount , MPI_FLOAT , source , MPI_COMM_WORLD ) ;

    printf ( "rank=%d results : %f %f %f %f \n", rank, rbuf[0], rbuf[1], rbuf[2], rbuf[3] ) ; 
}
else 
    printf ( "Must specify %d processors. Terminating. \n" , SIZE ) ;

    MPI_Finalize ( ) ;
}