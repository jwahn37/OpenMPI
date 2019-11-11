#include <sys/time.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>


int main()
{
   int num = 2;   //b10000000...
    int n=1;
   int i;
   //for( i=1; i<1024; i*=2)
    for(i=1; i<10; i++)
   {    
       num=i;
    //01000
   //while((num=num>>1 && 1) == 0)
    while((num & 1) == 0) 
        num = num>>1;
    if(num!=1)
        printf("not numberofcores! %d\n", num);
    else{
        printf("good!\n");
    }
   }
    return 0;
}
