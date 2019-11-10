#include <stdio.h>

typedef struct {
	unsigned char red,green,blue;
}PPMPixel;

int main()
{
    PPMPixel p[100];
    for(int i=0; i<100; i++)
    {
        p[i].red = 'r';
        p[i].green = 'g';
        p[i].blue = 'b';
    }

    for(int i=0; i<100; i++)
    {
        char *c = (char*)p;
        printf("%c", c[i]);
    }
    printf("\n");


}
