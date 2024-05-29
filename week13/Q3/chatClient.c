#include "headerFiles.h"

#define QUITMSG "quit\n"
#define PORT 8080
#define BUF_SIZE 256

int sock;

void handler(int signum)
{
    close(sock);
    exit(EXIT_SUCCESS);
}

int main()
{
    signal(SIGINT, handler);
    struct sockaddr_in addr;
    char buf[BUF_SIZE];

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1)
    {
        perror("sock");
        exit(1);
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(8080);

    if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) == -1)
    {
        perror("connect");
        exit(1);
    }

    while (1)
    {
        memset(buf, '\0', BUF_SIZE);
        printf("Enter: ");
        fgets(buf, BUF_SIZE, stdin);

        if (send(sock, buf, BUF_SIZE, 0) == -1)
        {
            perror("send");
            exit(1);
        }

        if (strcmp(buf, QUITMSG) == 0)
            break;
    }

    close(sock);
    return 0;
}