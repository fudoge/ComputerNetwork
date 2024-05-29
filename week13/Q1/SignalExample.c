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
    // SIGINT 시그널 핸들러를 등록
    signal(SIGINT, handler);

    // "Sleep begins!" 문자열 출력
    printf("Sleep begins!\n");

    // 1000초 동안 sleep
    sleep(1000);

    // "Wake up!" 문자열 출력
    printf("Wake up!\n");

    return 0;
}
