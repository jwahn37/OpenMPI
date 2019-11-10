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

int main()
{
	FILE *fp;
	PPMImage *img_ppm;
	PGMImage *img_pgm;
	char rfile_name[NAME_LEN];
	char wfile_name[NAME_LEN];
	int i,j;
	int w,h;
	int err=0;
	
	//파일을 읽어온다.
	scanf("%s", rfile_name);
	memcpy(wfile_name, rfile_name, sizeof(char)*NAME_LEN);
	wfile_name[strlen(wfile_name)-2] = 'g';	//pgm
	//printf("%s\n", wfile_name);
	//image 구조체 할당
	img_ppm = (PPMImage*) malloc (sizeof(PPMImage));

	err=PPM_file_read(rfile_name, img_ppm);
	if(err==-1)	return 0;

	//1. flip an image horizontally
	for(i=0; i<img_ppm->height; i++)
	{
		for(j=0; j<img_ppm->width/2; j++)
		{
			PPMPixel tp = img_ppm->pixels[i][j];
			img_ppm->pixels[i][j] = img_ppm->pixels[i][img_ppm->width-j-1];
			img_ppm->pixels[i][img_ppm->width-j-1] = tp;
		}
	}
	
	//2. reduce the image to grayscale by taking the advantage of the red, green, and blue value for each pixel.
	//https://gammabeta.tistory.com/391 공식

	//할당하기
	img_pgm = (PGMImage*)malloc(sizeof(PGMImage));
	memcpy(img_pgm, img_ppm, sizeof(PGMImage));	//image 메타데이터 읽어오기
	img_pgm->N = '5';
	printf("%s: check img new: %d %d %d %c %c\n", rfile_name, img_pgm->height, img_pgm->width, img_pgm->max, img_pgm->M, img_pgm->N);
	img_pgm->pixels = (PGMPixel**)malloc(sizeof(PGMPixel*)*img_pgm->height);
	for(i=0; i<img_pgm->height; i++)
	{
		img_pgm->pixels[i] = (PGMPixel*)malloc(sizeof(PGMPixel)*img_pgm->width);
	}

	//grey scale 계산하기. ITU-R BT.709 G=0.299R+0.587G+0.114B
	for(i=0; i<img_pgm->height; i++)
	{
		for(j=0; j<img_pgm->width;j++)
		{
			img_pgm->pixels[i][j].grey = 0.299*img_ppm->pixels[i][j].red + 0.587*img_ppm->pixels[i][j].green + 0.114*img_ppm->pixels[i][j].blue;
		}
	}

	//3. Smooth the image by calculating the mean of each pixel's value and ite eight neighbours (some algorithms consider only the values from the diagonal neighbours or the horizontal and vertical neighbours)

	//픽셀을 주변부와 평균화한다. (Average Fileter)
	//버퍼를 할당해서 카피해놓는다. (원본 훼손 방지)
	PGMPixel **tp_pgm = (PGMPixel**)malloc(sizeof(PGMPixel*)*img_pgm->height);
	for(int i=0; i<img_pgm->height; i++)
	{
		tp_pgm[i] = (PGMPixel*)malloc(sizeof(PGMPixel)*img_pgm->width);
	}
	memcpy(tp_pgm, img_pgm->pixels, sizeof(PGMPixel)*img_pgm->height*img_pgm->width);

	//평균화
	
	for(h=0; h<img_pgm->height; h++)
	{
		for(w=0; w<img_pgm->width; w++)
		{
			//여기서부터 주변부 검색하면서 평균화 한다.
			int sum_surr=0;
			for(int i=-1; i<=1; i++)
			{
				for(int j=-1; j<=1; j++)
				{
					if(h+i>=0 && h+i<img_pgm->height && w+j>=0 && w+j<img_pgm->width)	
					{
						sum_surr += tp_pgm[h+i][w+j].grey;
					}
					//아닐경우에는 0을 더하는게 일반적.
				}
			}
			//평균화하여 원본 이미지에 대입한다.
			img_pgm->pixels[h][w].grey = sum_surr/9;
		}
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
	img->pixels = (PPMPixel**)malloc(sizeof(PPMPixel*)*img->height);
	for(i=0; i<img->height; i++)
	{
		img->pixels[i] = (PPMPixel*)malloc(sizeof(PPMPixel)*img->width);
	}

	// <-- ppm 파일로부터 픽셀값을 읽어서 할당한 메모리에 load
	for(i=0; i<img->height; i++){
		for(j=0; j<img->width; j++){
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

	for(i=0; i<img->height; i++){
		for(j=0; j<img->width; j++){
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

	for(i=0; i<img->height; i++){
		for(j=0; j<img->width; j++){
			//fprintf(fp, "%c%c%c", img->pixels[i][j].red, img->pixels[i][j].green, img->pixels[i][j].blue);
			fprintf(fp, "%c", img->pixels[i][j].grey);
		}
	}
	fclose(fp);
}

int PPM_free(PPMImage *img)
{
	int i;
	for(i=0; i<img->height; i++)
	{
		free(img->pixels[i]);
	}
	free(img->pixels);
	free(img);
}

int PGM_free(PGMImage *img)
{
	int i;
	for(i=0; i<img->height; i++)
	{
		free(img->pixels[i]);
	}
	free(img->pixels);
	free(img);
}


