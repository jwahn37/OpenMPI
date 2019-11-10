#include <sys/time.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

__inline double GET_TIME(double now) { 
    struct timeval mytime; 
    // 현재 시간을 얻어온다.            
    gettimeofday(&mytime, NULL);    
    now = mytime.tv_sec + mytime.tv_usec / 1000000.0;   
}
int main()
{
    struct timeval mytime;

    // 현재 시간을 얻어온다.
    gettimeofday(&mytime, NULL);
    printf("%ld:%ld\n", mytime.tv_sec, mytime.tv_usec);

    // 시간을 1시간 뒤로 되돌려서 설정한다.
    mytime.tv_sec -= 3600;
    settimeofday(&mytime, NULL);
    
    return 0;
}