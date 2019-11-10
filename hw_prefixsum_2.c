#include <stdio.h> 
#include <stdlib.h>
#include "mpi.h"
#include "time.h"

//#define NUM_CORE 8
//#define COUNT 1

/*
NUM_CORE는 2의 지수승으로 가정
*/

int _log2(int num);
int _exp2(int num);
void print_psum(long long psum, int count, int *val);

int main ( int argc, char *argv[ ] )
{

    int numtasks, rank;
    int count;
    int i;
    long long psum;
    int level;
    long long revsum;
    int psize;
    int *val;
    MPI_Status st;
  
    MPI_Init ( &argc, &argv ) ;
    MPI_Comm_rank ( MPI_COMM_WORLD, &rank ) ; 
    MPI_Comm_size ( MPI_COMM_WORLD, &numtasks ) ;

    psize = atoi(argv[1]);
    count = psize/numtasks;
    val = (int*)malloc(sizeof(int)*count);

    srand(time(NULL)+rank); //make random variable
    
    psum=0;
    for(i=0; i<count; i++)
    {
        val[i] = (rank)*count+i+1;
        psum+=val[i];
    }
    /*
    for(i=0; i<COUNT; i++)
    {
        psum = rank+1;
        //sbuf[i] = rand()%100;
        //sbuf[i] = rank+1;
    }
*/
    /*
    for d = 1 to log2 n do
        for all k in parallel do
            if k >= 2^d  then
                x[out][k] = x[in][k – 2^(d-1)] + x[in][k]
            else
                x[out][k] = x[in][k]
    */

//    level = _log2(NUM_CORE);
    level = _log2(numtasks);

    for(i=0; i<level; i++)
    {
        //  printf("rank %d: %d\n", rank, _exp2(i));
        if(rank+_exp2(i)<numtasks)
            MPI_Send(&psum, 1, MPI_LONG_LONG, rank+_exp2(i), 0, MPI_COMM_WORLD);
            
        if(rank<_exp2(i))
            psum = psum;
        else
         //   psum[rank] += psum[rank-exp2(i)];
        {
            MPI_Recv(&revsum, 1, MPI_LONG_LONG, rank-_exp2(i), MPI_ANY_TAG, MPI_COMM_WORLD, &st);
            psum+=revsum;
        }
      
    }

    printf("rank=%d parital sum: ",rank);    
    print_psum(psum,count-1, val);
    printf("\n");
   // printf ( "rank=%d partial sum: %d\n", rank, psum) ; 

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
