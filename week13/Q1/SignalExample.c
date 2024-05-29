#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

// SIGINT 시그널 핸들러 함수
void handler(int signum)
{
    printf("Handler is called.\n");
    exit(EXIT_SUCCESS);
}

int main()
{
    signal(SIGINT, handler);
    printf("Sleep begins!\n");
    sleep(1000);
    
    printf("Wake up!\n");

    return 0;
}
