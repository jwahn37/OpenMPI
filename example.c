#include<stdio.h>

int main()
{
    int n=8;
    int v=0;
    int num;

    while(n>>=1)
        v++;
    printf("%d %d\n", n, v);

    num=8;
    v=1;
    while(num--)
        v*=2;
    printf("%d %d\n", num, v);
}

