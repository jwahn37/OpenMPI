#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NAME_LEN 40
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
	PPMPixel **pixels;
}PPMImage;

typedef struct {
	int width;
	int height;
	int max;
	char M, N;
	PGMPixel **pixels;
}PGMImage;

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

	stats = (MPI_Status*)malloc(sizeof(MPI_Status*numtasks));
	reqs = (MPI_Request*)malloc(sizeof(MPI_Request*numtasks));

	//파일을 읽어온다.
	scanf("%s", rfile_name);
	memcpy(wfile_name, rfile_name, sizeof(char)*NAME_LEN);
	wfile_name[strlen(wfile_name)-2] = 'g';	//pgm
	//printf("%s\n", wfile_name);
	//image 구조체 할당
	img_ppm = (PPMImage*) malloc (sizeof(PPMImage));

	err=PPM_file_read(rfile_name, img_ppm);
	if(err==-1)	return 0;

	//pgm 미리 할당
	//할당하기
	img_pgm = (PGMImage*)malloc(sizeof(PGMImage));
	memcpy(img_pgm, img_ppm, sizeof(PGMImage));	//image 메타데이터 읽어오기
	img_pgm->N = '5';
	printf("check img new: %d %d %d %c %c\n", img_pgm->height, img_pgm->width, img_pgm->max, img_pgm->M, img_pgm->N);
	img_pgm->pixels = (PGMPixel**)malloc(sizeof(PGMPixel*)*(img_pgm->height+2));
	for(i=0; i<img_pgm->height; i++)
	{
		img_pgm->pixels[i] = (PGMPixel*)malloc(sizeof(PGMPixel)*(img_pgm->width+2));
	}

	//0. MPI Derived data type 생성
	MPI_Datatype PPM_pixel_columns_type;
	MPI_Datatype PGM_pixel_columns_type;
	//MPI_Datatype PGM_pixel_3colums_type;

	MPI_Type_contiguous(img_ppm->width*3, MPI_CHAR, &PPM_pixel_columns_type);
	MPI_Type_commit(&PPM_pixel_columns_type);
	MPI_Type_contiguous(img_ppm->width, MPI_CHAR, &PGM_pixel_columns_type);
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
	
	//chunk_size = img_ppm->width * img_ppm->height / numtasks;
	chunk_size = img_ppm->height / numtasks;
	//Non-blocking으로 동작 시켜야 한다.
	for(i=0; i<numtasks; i++)
	{
		//MPI_Send(&psum, 1, MPI_INT, rank+_exp2(i), 0, MPI_COMM_WORLD);
		//rank : 0 , 1, 2, ..
		//최초줄은 0이기 떄문.
		//MPI_Isend(img_ppm->pixels[1+i*chunk_size],chunk_size, PPM_pixel_type, i, 0, MPI_COMM_WORLD, &req[i]);
		MPI_Isend(img_ppm->pixels[1+i*chunk_size],chunk_size, PPM_pixel_columns_type, i, 0, MPI_COMM_WORLD, &req[i]);
	}
	//MPI_Recv(&revsum, 1, MPI_INT, rank-_exp2(i), MPI_ANY_TAG, MPI_COMM_WORLD, &st);
	for(i=0; i<numtasks; i++)
	{
		MPI_Wait(&req[i], &stats[i]);
		MPI_Irecv(img_pgm->pixels[1+i*chunk_size], chunk_size, PGM_pixel_columns_type, i, MPI_ANY_TAG, MPI_COMM_WORLD, &req[i]);
	}

	//boundary 

	//3. Smooth the image by calculating the mean of each pixel's value and ite eight neighbours (some algorithms consider only the values from the diagonal neighbours or the horizontal and vertical neighbours)
	for(i=0; i<numtasks; i+++)
	{	
		MPI_Wait(&req[i], &stats[i]);
		if(i-1>=0)			MPI_Wait(&req[i-1], &stats[i-1]);
		if(i+1<numtasks)	MPI_Wait(&req[i+1], &stats[i+1]);

		MPI_Isend(img_pgm->pixels[i*chunk_size],chunk_size+2, PGM_pixel_columns_type, i, 0, MPI_COMM_WORLD, &req[i]);

	}

	for(i=0; i<numtasks; i++)
	{
		MPI_Wait(&req[i], &stats[i]);
		MPI_Irecv(img_pgm->pixels[1+i*chunk_size], chunk_size, PGM_pixel_columns_type, i, MPI_ANY_TAG, MPI_COMM_WORLD, &req[i]);
	}

	//파일을 쓴다.
	//err=PPM_file_write(wfile_name, img);
	err=PGM_file_write(wfile_name, img_pgm);
	if(err==-1)	return 0;

	PPM_free(img_ppm);
	//PPM_free(img_new);
	PGM_free(img_pgm);
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
	//printf("%d %d\n", img->width, img->height);
	fscanf(fp, "%d\n", &img->max);	


	if(img->max != 255){
		fprintf(stderr, "Image Format Error %d %d %d\n", img->width, img->height, img->max);
		return -1;
	}

	// <-- 메모리 할당
	img->pixels = (PPMPixel**)calloc(sizeof(PPMPixel*)*(img->height+2));
	for(i=0; i<img->height; i++)
	{
		img->pixels[i] = (PPMPixel*)calloc(sizeof(PPMPixel)*(img->width+2));
	}

	// <-- ppm 파일로부터 픽셀값을 읽어서 할당한 메모리에 load
	for(i=1; i<=img->height; i++){
		for(j=1; j<=img->width; j++){
			fscanf(fp, "%c%c%c", &(img->pixels[i][j].red), &(img->pixels[i][j].green), &(img->pixels[i][j].blue));
		}
	}
	fclose(fp);	
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
			fprintf(fp, "%c%c%c", img->pixels[i][j].red, img->pixels[i][j].green, img->pixels[i][j].blue);
		}
	}
	fclose(fp);
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
			fprintf(fp, "%c", img->pixels[i][j].grey);
		}
	}
	fclose(fp);
}

int PPM_free(PPMImage *img)
{
	int i;
	for(i=0; i<img->height+2; i++)
	{
		free(img->pixels[i]);
	}
	free(img->pixels);
	free(img);
}

int PGM_free(PGMImage *img)
{
	int i;
	for(i=0; i<img->height+2; i++)
	{
		free(img->pixels[i]);
	}
	free(img->pixels);
	free(img);
}

