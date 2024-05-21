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

void set_nonblocking(int sockfd) {
    int flags = fcntl(sockfd, F_GETFL, 0);
    if (flags == -1) {
        perror("fcntl F_GETFL");
        exit(EXIT_FAILURE);
    }

    if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) == -1) {
        perror("fcntl F_SETFL");
        exit(EXIT_FAILURE);
    }
}

int main() {
    int sockfd, newsockfd;
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t clilen;
    char buffer[BUFFER_SIZE];
    fd_set readfds;

    // 소켓 생성
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("ERROR opening socket");
        exit(EXIT_FAILURE);
    }

    // 소켓 주소 구조체 초기화
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(PORT);

    // 소켓 바인딩
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR on binding");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // 리슨 모드로 설정
    listen(sockfd, 5);
    printf("Server listening on port %d\n", PORT);

    clilen = sizeof(cli_addr);

    // 클라이언트 연결 수락
    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
    if (newsockfd < 0) {
        perror("ERROR on accept");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    printf("Accepted connection from client %s:%d\n", inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));

    // 소켓을 논블로킹 모드로 설정
    set_nonblocking(sockfd);
    set_nonblocking(newsockfd);
    set_nonblocking(STDIN_FILENO);  // 표준 입력을 논블로킹으로 설정

    while (1) {
        FD_ZERO(&readfds);
        FD_SET(newsockfd, &readfds);
        FD_SET(STDIN_FILENO, &readfds);

        int maxfd = (newsockfd > STDIN_FILENO) ? newsockfd : STDIN_FILENO;

        int activity = select(maxfd + 1, &readfds, NULL, NULL, NULL);
        if (activity < 0 && errno != EINTR) {
            perror("select error");
            close(newsockfd);
            close(sockfd);
            exit(EXIT_FAILURE);
        }

        // 클라이언트로부터 데이터 읽기
        if (FD_ISSET(newsockfd, &readfds)) {
            memset(buffer, 0, BUFFER_SIZE);
            int n = read(newsockfd, buffer, BUFFER_SIZE - 1);
            if (n < 0) {
                if (errno != EWOULDBLOCK && errno != EAGAIN) {
                    perror("ERROR reading from socket");
                    close(newsockfd);
                    close(sockfd);
                    exit(EXIT_FAILURE);
                }
            } else if (n == 0) {
                printf("Client disconnected\n");
                break;
            } else {
                printf("Client: %s\n", buffer);
            }
        }

        // 서버 입력 읽기
        if (FD_ISSET(STDIN_FILENO, &readfds)) {
            memset(buffer, 0, BUFFER_SIZE);
            int n = read(STDIN_FILENO, buffer, BUFFER_SIZE - 1);
            if (n < 0) {
                perror("ERROR reading from stdin");
                close(newsockfd);
                close(sockfd);
                exit(EXIT_FAILURE);
            }
            int m = write(newsockfd, buffer, n);
            if (m < 0) {
                if (errno != EWOULDBLOCK && errno != EAGAIN) {
                    perror("ERROR writing to socket");
                    close(newsockfd);
                    close(sockfd);
                    exit(EXIT_FAILURE);
                }
            }
        }
    }

    close(newsockfd);
    close(sockfd);
    return 0;
}
