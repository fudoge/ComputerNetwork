#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <errno.h>

#define PORT 12345
#define BUFFER_SIZE 256

void set_nonblocking(int sockfd)
{
    int flags = fcntl(sockfd, F_GETFL, 0);
    if (flags == -1)
    {
        perror("fcntl F_GETFL");
        exit(EXIT_FAILURE);
    }

    if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) == -1)
    {
        perror("fcntl F_SETFL");
        exit(EXIT_FAILURE);
    }
}

int main()
{
    int sockfd;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE];
    fd_set readfds;

    // 소켓 생성
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("ERROR opening socket");
        exit(EXIT_FAILURE);
    }

    // 서버 주소 구조체 초기화
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // 로컬 호스트
    serv_addr.sin_port = htons(PORT);

    // 서버에 연결 요청
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("ERROR connecting");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // 소켓을 논블로킹 모드로 설정
    set_nonblocking(sockfd);
    set_nonblocking(STDIN_FILENO); // 표준 입력을 논블로킹으로 설정

    while (1)
    {
        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);
        FD_SET(STDIN_FILENO, &readfds);

        int maxfd = (sockfd > STDIN_FILENO) ? sockfd : STDIN_FILENO;

        int activity = select(maxfd + 1, &readfds, NULL, NULL, NULL);
        if (activity < 0 && errno != EINTR)
        {
            perror("select error");
            close(sockfd);
            exit(EXIT_FAILURE);
        }

        // 서버로부터 데이터 읽기
        if (FD_ISSET(sockfd, &readfds))
        {
            memset(buffer, 0, BUFFER_SIZE);
            int n = read(sockfd, buffer, BUFFER_SIZE - 1);
            if (n < 0)
            {
                if (errno != EWOULDBLOCK && errno != EAGAIN)
                {
                    perror("ERROR reading from socket");
                    close(sockfd);
                    exit(EXIT_FAILURE);
                }
            }
            else if (n == 0)
            {
                printf("Server disconnected\n");
                break;
            }
            else
            {
                printf("Server: %s\n", buffer);
            }
        }

        // 클라이언트 입력 읽기
        if (FD_ISSET(STDIN_FILENO, &readfds))
        {
            memset(buffer, 0, BUFFER_SIZE);
            int n = read(STDIN_FILENO, buffer, BUFFER_SIZE - 1);
            if (n < 0)
            {
                perror("ERROR reading from stdin");
                close(sockfd);
                exit(EXIT_FAILURE);
            }
            int m = write(sockfd, buffer, n);
            if (m < 0)
            {
                if (errno != EWOULDBLOCK && errno != EAGAIN)
                {
                    perror("ERROR writing to socket");
                    close(sockfd);
                    exit(EXIT_FAILURE);
                }
            }
        }
    }

    close(sockfd);
    return 0;
}
