#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include "mpi.h"
#include "time.h"

///////////////////////////////////////////////////////////////////////////////////////////////////*/
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

void print_psum(long long psum, int count, int *val);
char Is_core_power2(int num);

int main ( int argc, char *argv[ ] )
{

    int numtasks, rank;
    int psize;
    int count;
    int i;
    long long psum;
    int *val;
    double start, finish;

    MPI_Init ( &argc, &argv ) ;
    MPI_Comm_rank ( MPI_COMM_WORLD, &rank ) ; 
    MPI_Comm_size ( MPI_COMM_WORLD, &numtasks ) ;

    psize = atoi(argv[1]);
    count = psize/numtasks;
    val = (int*)malloc(sizeof(int)*count);

    srand(time(NULL)+rank); //make random variable
    
    if(psize%numtasks!=0)
    {
        printf("Error: number should be a multiple of number of cores!\n");
        MPI_Finalize ( ) ;
        return 0;
    }
    if(!Is_core_power2(numtasks))
    {
        printf("Error: number of cores should be power of two!\n");
        MPI_Finalize ( ) ;
        return 0;
    }

    psum=0;
    for(i=0; i<count; i++)
    {
        //val[i] = (rank)*count+i+1;
        val[i] = rand() % 1000000 +1;
        psum+=val[i];
    }
    
    start = MPI_Wtime();

    MPI_Scan(&psum, &psum, 1, MPI_LONG_LONG,  MPI_SUM, MPI_COMM_WORLD);
   
    printf("rank=%d parital sum: ",rank);    
    print_psum(psum,count-1, val);
    printf("\n");
    
    finish = MPI_Wtime();
  //  printf("%e seconds from %d\n", finish-start, rank);
    MPI_Finalize ( ) ;
    
}

void print_psum(long long psum, int count, int *val)
{   
    if(count>=0) 
    {
        print_psum(psum-val[count], count-1, val);
        printf("%lld ", psum);
    }

}

char Is_core_power2(int num)
{
    while((num & 1) == 0) 
        num = num>>1;
    if(num!=1)
        return 0; //false
    else
        return 1; //true
}
