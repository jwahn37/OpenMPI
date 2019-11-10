#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mpi.h"

#define NAME_LEN 40
#define H 0
#define W 1
#define indexOf(i, j, width) (j+width*i)
/*
Image Processing using PPM format file
https://en.wikipedia.org/wiki/Netpbm_format
https://oneday0012.tistory.com/19

PGM, PPM
https://www.geeksforgeeks.org/c-program-to-write-an-image-in-pgm-format/
*/

typedef struct {
	unsigned char red,green,blue;
}PPMPixel;

typedef struct{
	unsigned char grey;
}PGMPixel;

typedef struct {
	int width;
	int height;
	int max;
	char M, N;
	//PPMPixel **pixels;
	PPMPixel *pixels;
}PPMImage;

typedef struct {
	int width;
	int height;
	int max;
	char M, N;
//	PGMPixel **pixels;
	PGMPixel *pixels;
}PGMImage;

int process_master(int rank, int numtasks);
int process_slaves(int rank, int numtasks);
int PPM_file_read(char file_name[NAME_LEN], PPMImage* img);
int PPM_file_write(char file_name[NAME_LEN], PPMImage *img);
int PGM_file_write(char file_name[NAME_LEN], PGMImage *img);
int PPM_free(PPMImage *img);
int PGM_free(PGMImage *img);

int main ( int argc, char *argv[ ] )
{
	FILE *fp;
	PPMImage *img_ppm;
	PGMImage *img_pgm;
	char rfile_name[NAME_LEN];
	char wfile_name[NAME_LEN];
	int i,j;
	int w,h;
	int err=0;
	int rank, numtasks;
	int chuk_size;
	MPI_Status *stats;
	MPI_Request *reqs;
	//MPI 초기화	
	MPI_Init ( &argc, &argv ) ;
    MPI_Comm_rank ( MPI_COMM_WORLD, &rank ) ; 
    MPI_Comm_size ( MPI_COMM_WORLD, &numtasks ) ;

	//printf("Hello %d\n", rank);
	
	//master
	if(rank==0)
	{
		err=process_master(rank, numtasks);
		if(err==-1)	printf("master node error \n");
	}
	//slaves
	else
	{
		err=process_slaves(rank, numtasks);
		if(err==-1)	printf("slave %d node error \n", rank);
	}
	
    MPI_Finalize ( ) ;

	return 0;
}


int process_master(int rank, int numtasks)
{
	FILE *fp;
	PPMImage *img_ppm;
	PGMImage *img_pgm;
	char rfile_name[NAME_LEN];
	char wfile_name[NAME_LEN];
	int i,j;
	int w,h;
	int err=0;
	//int rank, numtasks;
	int chunk_size;
	MPI_Status *stats;
	MPI_Request *reqs;


	stats = (MPI_Status*)malloc(sizeof(MPI_Status)*numtasks);
	reqs = (MPI_Request*)malloc(sizeof(MPI_Request)*numtasks);

	printf("I m a master\n");
	//scanf("%s", rfile_name);
	strcpy(rfile_name, "./ppm_example/Iggy.1024.ppm");
	//strcpy(rfile_name, "./ppm_example/small/sign_1.ppm");
	//추후 쓸 파일 이름 생성 (.ppm -> .pgm)
	memcpy(wfile_name, rfile_name, sizeof(char)*NAME_LEN);
	wfile_name[strlen(wfile_name)-2] = 'g';	//pgm
	//printf("%s\n", wfile_name);
	//image 구조체 할당
	
	//파일을 읽어온다.
	img_ppm = (PPMImage*) malloc (sizeof(PPMImage));
	err=PPM_file_read(rfile_name, img_ppm);
	if(err==-1)	return -1;

	//pgm 할당
	img_pgm = (PGMImage*)malloc(sizeof(PGMImage));
	memcpy(img_pgm, img_ppm, sizeof(PGMImage));	//image 메타데이터 읽어오기
	img_pgm->N = '5';
	printf("check img new: %d %d %d %c %c\n", img_pgm->height, img_pgm->width, img_pgm->max, img_pgm->M, img_pgm->N);
	
	/*
	img_pgm->pixels = (PGMPixel**)calloc(img_pgm->height+2, sizeof(PGMPixel*));
	for(i=0; i<=img_pgm->height+1; i++)
	{
		img_pgm->pixels[i] = (PGMPixel*)calloc(img_pgm->width+2, sizeof(PGMPixel));
	}
	*/
	img_pgm->pixels = (PGMPixel*)malloc(sizeof(PGMPixel)*(img_pgm->height+2)*(img_pgm->width+2));

	//0. MPI Derived data type 생성
	//각 타입은 PPM/PGM의 column을 나타내는 데이터 타입임.
	MPI_Datatype PPM_pixel_columns_type;
	MPI_Datatype PGM_pixel_columns_type;
	//MPI_Datatype PGM_pixel_3colums_type;

	MPI_Type_contiguous((img_ppm->width+2)*3, MPI_CHAR, &PPM_pixel_columns_type);
	MPI_Type_commit(&PPM_pixel_columns_type);
	MPI_Type_contiguous(img_ppm->width+2, MPI_CHAR, &PGM_pixel_columns_type);
	MPI_Type_commit(&PGM_pixel_columns_type);
	//MPI_Type_contiguous(img_ppm->width*3, MPI_CHAR, &PGM_pixel_3colums_type);
	//MPI_Type_commit(&PGM_pixel_3colums_type);
	//MPI_Datatype PPM_pixel_type;
	//MPI_Type_contiguous(3, MPI_CHAR, &PPM_pixel_type);
	//MPI_Type_commit(&PPM_pixel_type);

	//MPI_Datatype PGM_pixel_type;
	//MPI_Type_contiguous(1, MPI_CHAR, &PGM_pixel_type);
	//MPI_Type_commit(&PGM_pixel_type);
	//1. Ask slaves to do such things:
		//1.1. flip an image horizontally
		//1.2. reduce the image to grayscale by taking the advantage of the red, green, and blue value for each pixel.
	
	//Heigh, Width 정보를 보내야 한다.

	int hw[2];
	hw[H] = img_ppm->height;
	hw[W] = img_ppm->width;

	for(i=1; i<numtasks; i++)
	{
		MPI_Isend(hw,2, MPI_INT, i, 0, MPI_COMM_WORLD, &reqs[i]);
	}

	//chunk_size = img_ppm->width * img_ppm->height / numtasks;
	chunk_size = img_ppm->height / (numtasks-1);
	//Non-blocking으로 동작 시켜야 한다.
	for(i=1; i<numtasks; i++)	//master는 제외이므로 0는 빼라.
	{
		MPI_Wait(&reqs[i], &stats[i]);
		//MPI_Send(&psum, 1, MPI_INT, rank+_exp2(i), 0, MPI_COMM_WORLD);
		//rank : 0 , 1, 2, ..
		//최초줄은 0이기 떄문.
		//MPI_Isend(img_ppm->pixels[1+i*chunk_size],chunk_size, PPM_pixel_type, i, 0, MPI_COMM_WORLD, &req[i]);
		if(i==1)
			printf("master idx: %d, chunk: %d\n", 1+(i-1)*chunk_size, chunk_size);
		//MPI_Isend(&(img_ppm->pixels[(1+(i-1)*chunk_size)*(img_ppm->width+2)]),chunk_size, PPM_pixel_columns_type, i, 0, MPI_COMM_WORLD, &reqs[i]);
		MPI_Isend(&(img_ppm->pixels[indexOf(1+(i-1)*chunk_size, 0, img_ppm->width+2)]),chunk_size, PPM_pixel_columns_type, i, 0, MPI_COMM_WORLD, &reqs[i]);
	}
	//MPI_Recv(&revsum, 1, MPI_INT, rank-_exp2(i), MPI_ANY_TAG, MPI_COMM_WORLD, &st);
	printf("master check 5\n");

	for(i=1; i<numtasks; i++)
	{
		MPI_Wait(&reqs[i], &stats[i]);
		///MPI_Irecv(img_pgm->pixels[1+(i-1)*chunk_size], chunk_size, PGM_pixel_columns_type, i, MPI_ANY_TAG, MPI_COMM_WORLD, &reqs[i]);
		MPI_Irecv(&(img_pgm->pixels[indexOf(1+(i-1)*chunk_size, 0, img_pgm->width+2)]), chunk_size, PGM_pixel_columns_type, i, MPI_ANY_TAG, MPI_COMM_WORLD, &reqs[i]);
	}

	//boundary 
/*
	//3. Smooth the image by calculating the mean of each pixel's value and ite eight neighbours (some algorithms consider only the values from the diagonal neighbours or the horizontal and vertical neighbours)
	for(i=1; i<numtasks; i+++)
	{	
		MPI_Wait(&req[i], &stats[i]);	//본인도 기다리고
		if(i-1>=0)			MPI_Wait(&req[i-1], &stats[i-1]);	//한번 이전과,
		if(i+1<numtasks)	MPI_Wait(&req[i+1], &stats[i+1]);	//한번 이후도 기다려야함.

		MPI_Isend(img_pgm->pixels[(i-1)*chunk_size],chunk_size+2, PGM_pixel_columns_type, i, 0, MPI_COMM_WORLD, &req[i]);

	}

	for(i=1; i<numtasks; i++)
	{
		MPI_Wait(&req[i], &stats[i]);
		MPI_Irecv(img_pgm->pixels[1+(i-1)*chunk_size], chunk_size, PGM_pixel_columns_type, i, MPI_ANY_TAG, MPI_COMM_WORLD, &req[i]);
	}
*/
printf("master check 6\n");

	//파일을 쓴다.
	//err=PPM_file_write(wfile_name, img);
	err=PGM_file_write(wfile_name, img_pgm);
	if(err==-1)	return -1;
printf("master check 7\n");

	PPM_free(img_ppm);
	//PPM_free(img_new);
	PGM_free(img_pgm);

	free(stats);
	free(reqs);
printf("master check 8\n");

	return 0;
}

int process_slaves(int rank, int numtasks)
{
	int hw[2];
	int height, width;
	MPI_Status st;
	MPI_Request req;
	//PPMPixel **ppm_pixels;
	//PGMPixel **pgm_pixels;
	PPMPixel* ppm_pixels;
	PGMPixel* pgm_pixels;
	
	int i, j;
	int chunk_size;

	//0. MPI Derived data type 생성
	//각 타입은 PPM/PGM의 column을 나타내는 데이터 타입임.
	MPI_Datatype PPM_pixel_columns_type;
	MPI_Datatype PGM_pixel_columns_type;
	//MPI_Datatype PGM_pixel_3colums_type;

	
	//호스트로부터 HEIGHT, WIDTH정보 받아온다.
	MPI_Recv(hw, 2, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &st);
	height = hw[H];
	width = hw[W];

	//0. MPI Derived data type 생성
	//각 타입은 PPM/PGM의 column을 나타내는 데이터 타입임.
	MPI_Type_contiguous((width+2)*3, MPI_CHAR, &PPM_pixel_columns_type);
	MPI_Type_commit(&PPM_pixel_columns_type);
	MPI_Type_contiguous(width+2, MPI_CHAR, &PGM_pixel_columns_type);
	MPI_Type_commit(&PGM_pixel_columns_type);

	//연산할 ppm, pgm 할당 (1024/8=128)
	chunk_size = height / (numtasks-1);
	if(rank==1)
	{
		printf("slave w, h, chunk: %d %d %d\n", width, height, chunk_size);
	}

	//2차원배열과 2차원포인터는 메모리할당이 전혀 다르다!!!!! 
	//ppm_pixels = (PPMPixel**)malloc(sizeof(PPMPixel*)*(chunk_size));
	//pgm_pixels = (PGMPixel**)malloc(sizeof(PGMPixel*)*(chunk_size));
	/*
	ppm_pixels = (PPMPixel**)calloc(chunk_size, sizeof(PPMPixel*));
	pgm_pixels = (PGMPixel**)calloc(chunk_size, sizeof(PGMPixel*));	

	for(i=0; i<chunk_size; i++)
	{
		ppm_pixels[i] = (PPMPixel*)calloc(width+2, sizeof(PPMPixel));	//1~WIDTH까지만 실제수, 0,WIDTH는 0화되있음.
		pgm_pixels[i] = (PGMPixel*)calloc(width+2, sizeof(PGMPixel));	//1~WIDTH까지만 실제수, 0,WIDTH는 0화되있음.
	}
*/	
	//반드시 1차원으로 할당해야 데이터를 연속적으로 받아올 수 있다. 
	ppm_pixels = (PPMPixel*)malloc(sizeof(PPMPixel)*chunk_size*(width+2));
	pgm_pixels = (PGMPixel*)malloc(sizeof(PGMPixel)*chunk_size*(width+2));

	memset(ppm_pixels, 0x00, sizeof(PPMPixel)*chunk_size*(width+2));
	memset(ppm_pixels, 0x00, sizeof(PGMPixel)*chunk_size*(width+2));

	//1. 호스트로부터 연산할 ppm 받아온다.

	//MPI_Recv(ppm_pixels, chunk_size, PPM_pixel_columns_type, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &st);
	MPI_Irecv(ppm_pixels, chunk_size, PPM_pixel_columns_type, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &req);
	//MPI_Irecv(img_pgm->pixels[1+(i-1)*chunk_size], chunk_size, PGM_pixel_columns_type, i, MPI_ANY_TAG, MPI_COMM_WORLD, &reqs[i]);

	
	//1.1. flip an image horizontally
	for(i=0; i<chunk_size; i++)
	{
		for(j=1; j<=width/2; j++)
		{
			//PPMPixel tp = ppm_pixels[i][j];
			//ppm_pixels[i][j] = ppm_pixels[i][width-j+1];	//1~WIDTH까지가 범위임
			//ppm_pixels[i][width-j+1] = tp;

			PPMPixel tp = ppm_pixels[indexOf(i, j, width+2)];
			ppm_pixels[indexOf(i, j, width+2)] = ppm_pixels[indexOf(i,width-j+1, width+2)];	//1~WIDTH까지가 범위임
			ppm_pixels[indexOf(i,width-j+1, width+2)] = tp;
		}
	}	
	
	//1.2. reduce the image to grayscale by taking the advantage of the red, green, and blue value for each pixel.
	for(i=0; i<chunk_size; i++)
	{
		for(j=1; j<=width; j++)
		{
			pgm_pixels[indexOf(i, j, width+2)].grey = 0.299*ppm_pixels[indexOf(i, j, width+2)].red + 0.587*ppm_pixels[indexOf(i, j, width+2)].green + 0.114*ppm_pixels[indexOf(i, j, width+2)].blue;
		//	pgm_pixels[i][j].grey = 0.299*ppm_pixels[i][j].red + 0.587*ppm_pixels[i][j].green + 0.114*ppm_pixels[i][j].blue;
		}
	}	
	
	//호스트에게 pgm을 전송한다.
   // MPI_Send(pgm_pixels, chunk_size, PGM_pixel_columns_type, 0, 0, MPI_COMM_WORLD);

	//2. 호스트로부터 Average filetering을 수행하기 위한 데이터를 받아온다..

	return 0;


}
int PPM_file_read(char file_name[NAME_LEN], PPMImage* img)
{
	FILE *fp;
	int i,j;
	char hashtag[100];

	fp = fopen(file_name, "rb");	
	
	if(fp == NULL){
		fprintf(stderr, "file open error: %s\n", file_name);
		return -1;
	}
	
	fscanf(fp, "%c%c\n", &img->M, &img->N);	
	if(img->M != 'P' || img->N != '6'){
		fprintf(stderr, "Image Format Error: %c%c\n", img->M, img->N);
		return -1;
	}
	
	//fgets(hashtag, 1000, fp);
	//printf("%s\n", hashtag);

	int res;
	res=fscanf(fp, "%d %d\n", &img->width, &img->height);	
	//printf("res %d\n", res);
	if(res==0)	//something bad happens
	{
		char comment_msg[100];
		fgets(comment_msg, 100, fp);
	//	printf("%s\n", comment_msg);
		fscanf(fp, "%d %d\n", &img->width, &img->height);	
	}

	fscanf(fp, "%d\n", &img->max);	


	if(img->max != 255){
		fprintf(stderr, "Image Format Error %d %d %d\n", img->width, img->height, img->max);
		return -1;
	}


	// <-- 메모리 할당
	/*
	img->pixels = (PPMPixel**)calloc(img->height+2, sizeof(PPMPixel*));
	for(i=0; i<=img->height+1; i++)
	{
		img->pixels[i] = (PPMPixel*)calloc(img->width+2, sizeof(PPMPixel));
	}
	*/
	img->pixels = (PPMPixel*)malloc(sizeof(PPMPixel)*(img->height+2)*(img->width+2));
	memset(img->pixels, 0x00, sizeof(PPMPixel)*(img->height+2)*(img->width+2));
	
	// <-- ppm 파일로부터 픽셀값을 읽어서 할당한 메모리에 load
	//int res;
	for(i=1; i<=img->height; i++){
		for(j=1; j<=img->width; j++){
			//res=fscanf(fp, "%c%c%c", &(img->pixels[i][j].red), &(img->pixels[i][j].green), &(img->pixels[i][j].blue));
			res=fscanf(fp, "%c%c%c", &(img->pixels[indexOf(i,j,img->width+2)].red), &(img->pixels[indexOf(i,j,img->width+2)].green), &(img->pixels[indexOf(i,j,img->width+2)].blue));
		}
	}
	
	fclose(fp);	

	return 0;
}



int PPM_file_write(char file_name[NAME_LEN], PPMImage *img)
{
	FILE* fp;
	int i,j;

	fp = fopen(file_name, "wb");
	
	if(img->M != 'P' || img->N != '6'){
		fprintf(stderr, "Image Format Error: %c%c\n", img->M, img->N);
		return -1;
	}

	fprintf(fp, "%c%c\n", img->M, img->N);	
	fprintf(fp, "%d %d\n", img->width, img->height);	
	fprintf(fp, "%d\n", img->max);

	for(i=1; i<=img->height; i++){
		for(j=1; j<=img->width; j++){
			//fprintf(fp, "%c%c%c", img->pixels[i][j].red, img->pixels[i][j].green, img->pixels[i][j].blue);
			fprintf(fp, "%c%c%c", img->pixels[indexOf(i,j,img->width+2)].red, img->pixels[indexOf(i,j,img->width+2)].green, img->pixels[indexOf(i,j,img->width+2)].blue);
		}
	}
	fclose(fp);

	return 0;
}

int PGM_file_write(char file_name[NAME_LEN], PGMImage *img)
{
	FILE* fp;
	int i,j;

	fp = fopen(file_name, "wb");
	
	if(img->M != 'P' || img->N != '5'){
		fprintf(stderr, "Image Format Error: %c%c\n", img->M, img->N);
		return -1;
	}

	fprintf(fp, "%c%c\n", img->M, img->N);	
	fprintf(fp, "%d %d\n", img->width, img->height);	
	fprintf(fp, "%d\n", img->max);

	for(i=1; i<=img->height; i++){
		for(j=1; j<=img->width; j++){
			//fprintf(fp, "%c%c%c", img->pixels[i][j].red, img->pixels[i][j].green, img->pixels[i][j].blue);
		//	fprintf(fp, "%c", img->pixels[i][j].grey);
			fprintf(fp, "%c", img->pixels[indexOf(i,j,img->width+2)].grey);
		}
	}
	fclose(fp);

	return 0;
}

int PPM_free(PPMImage *img)
{
	int i;
	/*
	for(i=0; i<img->height+2; i++)
	{
		free(img->pixels[i]);
	}
	free(img->pixels);
	*/
	//for(i=0; i<(img->height+2)*(img->width+2); i++)
	//	free(img->pixels[i]);
	free(img->pixels);
	free(img);
	return 0;
}

int PGM_free(PGMImage *img)
{
	int i;
//	for(i=0; i<img->height+2; i++)
//	{
//		free(img->pixels[i]);
//	}
	free(img->pixels);
	free(img);
	return 0;
}
