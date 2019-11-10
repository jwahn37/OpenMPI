#include <stdio.h>
#include <mpi.h>
#include <string.h>

#define MAX_LEN 100
#define MESSTAG 1

int main(int argc, char *argv[]) {
	int rank, num, i;

	MPI_Init(&argc, &argv); 
	MPI_Comm_size(MPI_COMM_WORLD, &num); 
	MPI_Comm_rank(MPI_COMM_WORLD, &rank); 
	
    if(rank==0){
        char mess[] = "Hello World";
        int len = strlen(mess)+1;
        for(i=1; i<num; i++)
            MPI_Send(mess, len, MPI_CHAR, i, MESSTAG, MPI_COMM_WORLD);
    }
    else{
        char mess[MAX_LEN];
        MPI_Status status;
        MPI_Recv(mess, MAX_LEN, MPI_CHAR, 0, MESSTAG, MPI_COMM_WORLD, &status);
        printf("%i received %s\n", rank, mess);
    }	
	MPI_Finalize();
}
