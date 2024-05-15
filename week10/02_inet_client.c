#include "headerFiles.h"

#define PORT 8000

int main() {
    int sock;
    struct sockaddr_in addr;
    char buf_recv[1000];
    char buf_send[1000];
    char *quit = "\\quit";
    socklen_t buf_len;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(PORT);

    if(connect(sock, (const struct sockaddr *)&addr, sizeof(addr)) == -1) {
        perror("connect");
        exit(1);
    }

    while(1) {
        scanf("%s", buf_send);

        if(send(sock, buf_send, sizeof(buf_send), 0) == -1) {
            perror("send");
            exit(1);
        }
        if(strcmp(buf_send, quit) == 0) {
            break;
        }

        buf_len = recv(sock, buf_recv, sizeof(buf_recv), 0);
        if(buf_len < 0) {
            perror("recv");
            exit(1);
        }

        if(strcmp(buf_recv, quit) == 0) {
            break;
        }

        printf("[You] %s\n", buf_recv);
    }
    close(sock);
    return 0;
}