#include <stdio.h> 
#include "mpi.h"
#include "time.h"

#define NUM_CORE 8

/*
질문사항 1.
n=number of core?
*/

int main ( int argc, char *argv[ ] )
{

    int numtasks, rank;
    int count;
    int i;
   // int sbuf[COUNT], rbuf[COUNT];
    int send_, rev_;
   //int sendcount, recvcount, source;  
    //float sbuf[SIZE][SIZE] = { { 1.0 , 2.0 , 3.0 , 4.0 } , { 5.0 , 6.0 , 7.0 , 8.0 } , { 9.0 , 10.0 , 11.0 , 12.0 } , { 13.0 , 14.0 , 15.0 , 16.0 }} ;
    //float rbuf[SIZE] ;
    //int sbuf[SIZE];

    

    MPI_Init ( &argc, &argv ) ;
    MPI_Comm_rank ( MPI_COMM_WORLD, &rank ) ; 
    MPI_Comm_size ( MPI_COMM_WORLD, &numtasks ) ;

    srand(time(NULL)+rank); //make random variable
  //  for(i=0; i<COUNT; i++)
  //  {
     //   sbuf[i] = rand()%100;
        //sbuf[i] = rank+1;
        send_ = rank+1;
   // }
    // MPI_Scatter ( sbuf , sendcount , MPI_FLOAT , rbuf , recvcount , MPI_FLOAT , source , MPI_COMM_WORLD ) ; 
    /*///////////////////////////////////////////////////////////////////////////////////////////////////
    int MPI_Scan(const void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, MPI_Op op, MPI_Comm comm)

    Description
    MPI_SCAN is used to perform a prefix reduction on data distributed across the group. The operation returns, in the receive buffer of the process with rank i, the reduction of the values in the send buffers of processes with ranks 0,...,i (inclusive). The type of operations supported, their semantics, and the constraints on send and receive buffers are as for MPI_REDUCE.
    
    input parameter
    sendbuf: starting address of send buffer (choice)
    count: number of elements in input buffer (integer)
    datatype: data type of elements of input buffer (handle)
    op: operation (handle)
    comm: communicator (handle)

    output parameter
    recvbuf: starting address of receive buffer (choice)

    ///////////////////////////////////////////////////////////////////////////////////////////////////*/

    MPI_Scan(&send_, &rev_, COUNT, MPI_INT,  MPI_SUM, MPI_COMM_WORLD);
    //    printf ( "rank=%d results : %f %f %f %f \n", rank, rbuf[0], rbuf[1], rbuf[2], rbuf[3] ) ; 
    printf ("rank=%d partial sum: %d\n", rank, rev_); 

    MPI_Finalize ( ) ;
}
