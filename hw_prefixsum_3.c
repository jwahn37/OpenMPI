#include <stdio.h> 
#include "mpi.h"
#include "time.h"

#define NUM_CORE 8
#define COUNT 1

/*
NUM_CORE는 2의 지수승으로 가정
https://developer.nvidia.com/gpugems/GPUGems3/gpugems3_ch39.html
Example 3 잘못 표기됨 2^d+1 이 아니라, 2^(d+1)
Work Effiecient Parallel Scan
using balanced binary tree
*/

int _log2(int num);
int _exp2(int num);

int main ( int argc, char *argv[ ] )
{

    int numtasks, rank;
    int count;
    int d,k,i,t;
    int psum;
    int psum_;
    int level;
    int revsum;
    MPI_Status st;
  
    MPI_Init ( &argc, &argv ) ;
    MPI_Comm_rank ( MPI_COMM_WORLD, &rank ) ; 
    MPI_Comm_size ( MPI_COMM_WORLD, &numtasks ) ;

    srand(time(NULL)+rank); //make random variable
    for(i=0; i<COUNT; i++)
    {
        psum = rank+1;
        //sbuf[i] = rand()%100;
        //sbuf[i] = rank+1;
    }


    level = _log2(NUM_CORE);

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
            if( rank+_exp2(d) < NUM_CORE)
                MPI_Send(&psum, 1, MPI_INT, rank+_exp2(d), 0, MPI_COMM_WORLD);
        }
        if( (rank+1) % _exp2(d+1) == 0 )    //이전 버전으로부터 받아와서 더한다.
        {
            MPI_Recv(&revsum, 1, MPI_INT, rank-_exp2(d), MPI_ANY_TAG, MPI_COMM_WORLD, &st);
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

    if(rank==NUM_CORE-1)
    {
        psum_ = psum;
        psum = 0;
    }    

    for(d=level-1; d>=0; d--)
    {
        if( (rank+1) % _exp2(d) == 0 && (rank+1) % _exp2(d+1) != 0 )    //rank+1이 2^d의 배수이면서 2^(d+1)의 배수는 아닐 떄 -> send해야함 
        {
            if( rank+_exp2(d) < NUM_CORE)
            {
                MPI_Send(&psum, 1, MPI_INT, rank+_exp2(d), 0, MPI_COMM_WORLD);
                MPI_Recv(&psum, 1, MPI_INT, rank+_exp2(d), MPI_ANY_TAG, MPI_COMM_WORLD, &st);
            }
            //if( rank+_exp2(d) < NUM_CORE)
            //    MPI_Send(&psum, 1, MPI_INT, rank+_exp2(d), 0, MPI_COMM_WORLD);
        }
        if( (rank+1) % _exp2(d+1) == 0 )    //이전 버전으로부터 받아와서 더한다.
        {
            MPI_Recv(&revsum, 1, MPI_INT, rank-_exp2(d), MPI_ANY_TAG, MPI_COMM_WORLD, &st);
            MPI_Send(&psum, 1, MPI_INT, rank-_exp2(d), 0, MPI_COMM_WORLD);
            psum += revsum;
        }
    }
    
    if(rank==NUM_CORE-1)
    {
        MPI_Send(&psum_, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }
    if(rank==0)
    {
        MPI_Recv(&psum, 1, MPI_INT, NUM_CORE-1, MPI_ANY_TAG, MPI_COMM_WORLD, &st);
    }

    printf ( "rank=%d partial sum: %d\n", rank, psum) ; 

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
