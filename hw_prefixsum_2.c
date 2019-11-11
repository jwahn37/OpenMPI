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
char Is_core_power2(int num);

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
    double start, finish;

    MPI_Init ( &argc, &argv ) ;
    MPI_Comm_rank ( MPI_COMM_WORLD, &rank ) ; 
    MPI_Comm_size ( MPI_COMM_WORLD, &numtasks ) ;

    psize = atoi(argv[1]);
    count = psize/numtasks;
    val = (int*)malloc(sizeof(int)*count);

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


    srand(time(NULL)+rank); //make random variable
    
    psum=0;
    for(i=0; i<count; i++)
    {
        //val[i] = (rank)*count+i+1;
        val[i] = rand() % 1000000 +1;

        psum+=val[i];
    }
   
    /*
    for d = 1 to log2 n do
        for all k in parallel do
            if k >= 2^d  then
                x[out][k] = x[in][k – 2^(d-1)] + x[in][k]
            else
                x[out][k] = x[in][k]
    */

    start = MPI_Wtime();

    level = _log2(numtasks);

    for(i=0; i<level; i++)
    {
        if(rank+_exp2(i)<numtasks)
            MPI_Send(&psum, 1, MPI_LONG_LONG, rank+_exp2(i), 0, MPI_COMM_WORLD);
            
        if(rank<_exp2(i))
            psum = psum;
        else
        {
            MPI_Recv(&revsum, 1, MPI_LONG_LONG, rank-_exp2(i), MPI_ANY_TAG, MPI_COMM_WORLD, &st);
            psum+=revsum;
        }
      
    }

    printf("rank=%d parital sum: ",rank);    
    print_psum(psum,count-1, val);
    printf("\n");
    
    finish = MPI_Wtime();
   // printf("%e seconds from %d\n", rank, finish-start);

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

char Is_core_power2(int num)
{
    while((num & 1) == 0) 
        num = num>>1;
    if(num!=1)
        return 0; //false
    else
        return 1; //true
}

