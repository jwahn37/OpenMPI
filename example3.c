#include <stdio.h>

typedef struct {
	unsigned char red,green,blue;
}PPMPixel;

int main()
{
    PPMPixel p[100];
    int i;
    for(i=0; i<100; i++)
    {
        p[i].red = 'r';
        p[i].green = 'g';
        p[i].blue = 'b';
    }

    for(i=0; i<100; i++)
    {
        char *c = (char*)p;
        printf("%c", c[i]);
    }
    for(i=0; i<100; i++)
    {
        printf("%d\n", p+i);
    }
    printf("\n");


}
