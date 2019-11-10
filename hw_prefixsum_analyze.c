#include<stdio.h>
#include <stdlib.h>

#define LINE_LEN 60
#define E 8
#define D 10
#define F 12
int main()
{
    char line[LINE_LEN];
    double sec;
    int dec;
    double sum;
    double ave_sum;
    int nc;

    sum=0;
    nc=0;
    while(fgets(line, 60, stdin))
    {
        nc++;
        if(line[E]=='e')
        {
            line[E]='\0';
            sec=atof(line);
            line[F]='\0';
            dec=atoi(&line[D]);
       
        }
        while(dec--) sec/=10;
       // printf("%lf\n",sec);
        sum+=sec;
        //printf("%s %lf\n",line, sec);

    }
    ave_sum = sum/nc;
    printf("cores: %d execution time (amount, avarage): %lf %lf\n", nc, sum, ave_sum);
}
