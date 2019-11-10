#include <stdio.h> 
#include <stdlib.h>
#include "mpi.h"
#include "time.h"

//#define numtasks 8
//#define COUNT 1

/*
https://developer.nvidia.com/gpugems/GPUGems3/gpugems3_ch39.html
Example 3 잘못 표기됨 2^d+1 이 아니라, 2^(d+1)
Work Effiecient Parallel Scan
using balanced binary tree
*/

int _log2(int num);
int _exp2(int num);
void print_psum(long long psum, int count, int *val);

int main ( int argc, char *argv[ ] )
{

    int numtasks, rank;
    int count;
    int d,k,i,t;
    long long psum;
    long long psum_;
    int level;
    long long revsum;
    MPI_Status st;
    int *val, *val_pvnode;

    int psize;
  
    MPI_Init ( &argc, &argv ) ;
    MPI_Comm_rank ( MPI_COMM_WORLD, &rank ) ; 
    MPI_Comm_size ( MPI_COMM_WORLD, &numtasks ) ;

    psize = atoi(argv[1]);
    count = psize/numtasks;
    val = (int*)malloc(sizeof(int)*count);
    val_pvnode = (int*)malloc(sizeof(int)*count);

    srand(time(NULL)+rank); //make random variable
    psum=0;
    for(i=0; i<count; i++)
    {
        val[i] = (rank)*count+i+1;
        psum+=val[i];
    }

    level = _log2(numtasks);

    /* 
    Up-Sweep (Reduce) Phase
    for d = 0 to log2 n – 1 do
        for all k = 0 to n – 1 by 2^(d+1) in parallel do
            x[k +  2^(d+1) – 1] = x[k +  2^d  – 1] + x[k +  2^(d+1) – 1]
    */

    for(d=0; d<level; d++)
    {
        if( (rank+1) % _exp2(d) == 0 && (rank+1) % _exp2(d+1) != 0 )    //rank+1이 2^d의 배수이면서 2^(d+1)의 배수는 아닐 떄 -> send해야함 
        {
            if( rank+_exp2(d) < numtasks)
                MPI_Send(&psum, 1, MPI_LONG_LONG, rank+_exp2(d), 0, MPI_COMM_WORLD);
        }
        if( (rank+1) % _exp2(d+1) == 0 )    //이전 버전으로부터 받아와서 더한다.
        {
            MPI_Recv(&revsum, 1, MPI_LONG_LONG, rank-_exp2(d), MPI_ANY_TAG, MPI_COMM_WORLD, &st);
            psum += revsum;
        }
    }
    
    //printf ( "rank=%d partial sum: %d\n", rank, psum) ; 

    /*
    Down-Sweep Phase 
    x[n – 1] <- 0
    for d = log2 n – 1 down to 0 do
         for all k = 0 to n – 1 by 2^(d+1) in parallel do
             t = x[k +  2^d  – 1]
              x[k +  2^d  – 1] = x[k +  2^(d +1) – 1]
              x[k +  2^(d+1) – 1] = t +  x[k +  2^(d+1) – 1]
    */

    if(rank==numtasks-1)
    {
        psum_ = psum;
        psum = 0;
    }    

    for(d=level-1; d>=0; d--)
    {
        if( (rank+1) % _exp2(d) == 0 && (rank+1) % _exp2(d+1) != 0 )    //rank+1이 2^d의 배수이면서 2^(d+1)의 배수는 아닐 떄 -> send해야함 
        {
            if( rank+_exp2(d) < numtasks)
            {
                MPI_Send(&psum, 1, MPI_LONG_LONG, rank+_exp2(d), 0, MPI_COMM_WORLD);
                MPI_Recv(&psum, 1, MPI_LONG_LONG, rank+_exp2(d), MPI_ANY_TAG, MPI_COMM_WORLD, &st);
            }
            //if( rank+_exp2(d) < numtasks)
            //    MPI_Send(&psum, 1, MPI_INT, rank+_exp2(d), 0, MPI_COMM_WORLD);
        }
        if( (rank+1) % _exp2(d+1) == 0 )    //이전 버전으로부터 받아와서 더한다.
        {
            MPI_Recv(&revsum, 1, MPI_LONG_LONG, rank-_exp2(d), MPI_ANY_TAG, MPI_COMM_WORLD, &st);
            MPI_Send(&psum, 1, MPI_LONG_LONG, rank-_exp2(d), 0, MPI_COMM_WORLD);
            psum += revsum;
        }
    }
    
    if(rank==numtasks-1)
    {
        MPI_Send(&psum_, 1, MPI_LONG_LONG, 0, 0, MPI_COMM_WORLD);
    }
    if(rank==0)
    {
        MPI_Recv(&psum, 1, MPI_LONG_LONG, numtasks-1, MPI_ANY_TAG, MPI_COMM_WORLD, &st);
    }

    //*value가 한번씩 옆으로 회전됐으므로 가져와야함.
    int prev = rank-1;
    int next = rank+1;
    MPI_Request reqs[2];
    MPI_Status stats[2];
    int tagS=1;
    int tagR=2;
    if(rank==0)                 prev = numtasks-1;
    if(rank == (numtasks-1))    next = 0;


    if(count>1)
    {
        ///tag = prev
        //(buf, count, datatype, source, tag, comm, req)
        MPI_Irecv(val_pvnode, count, MPI_INT, prev, MPI_ANY_TAG, MPI_COMM_WORLD, &reqs[0]);
        //tag = rank
        MPI_Isend(val, count, MPI_INT, next, 0, MPI_COMM_WORLD, &reqs[1]);

        MPI_Waitall(2, reqs, stats);
    }


   // printf ( "rank=%d partial sum: %d\n", rank, psum) ; 
    printf("rank=%d parital sum: ",rank);    
    print_psum(psum,count-1, val_pvnode);
   // print_psum(psum,count-1, val);
    printf("\n");

    MPI_Finalize ( ) ;
}

int _log2(int num)
{
    int v=0;
    while(num>>=1)
        v++;
    return v;
}

int _exp2(int num)
{
    int v=1;
    while(num--)
        v*=2;
    return v;
}

void print_psum(long long psum, int count, int *val)
{   
    //psum_r -= val[count];
    if(count>=0) 
    {
        print_psum(psum-val[count], count-1, val);
        printf("%lld ", psum);
    }

}
