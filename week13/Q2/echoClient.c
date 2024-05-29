#include "headerFiles.h"

#define PORT 8080
#define BUF_SIZE 256
#define QUIT "quit\n"

int sock;

void handler(int signum)
{
    close(sock);
    exit(EXIT_SUCCESS);
}

int main()
{
    signal(SIGINT, handler);
    sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;
    char buf[BUF_SIZE];

    if (sock == -1)
    {
        perror("socket");
        exit(1);
    }

    memset(&addr, '\0', sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(PORT);

    if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) == -1)
    {
        perror("connect");
        close(sock);
        exit(1);
    }

    if (recv(sock, buf, sizeof(buf), 0) < 0)
    {
        perror("recv");
        close(sock);
        exit(1);
    }

    printf("%s\n", buf);

    while (1)
    {
        printf("Enter message: ");
        if (fgets(buf, sizeof(buf), stdin) != NULL)
        {
            if (send(sock, buf, strlen(buf) + 1, 0) == -1)
            {
                perror("send");
                close(sock);
                exit(1);
            }

            if (strcmp(buf, QUIT) == 0)
            {
                close(sock);
                exit(1);
            }

            if (recv(sock, buf, sizeof(buf), 0) < 0)
            {
                perror("recv");
                close(sock);
                exit(1);
            }
            printf("Recv from server:\n%s\n", buf);
        }
    }

    close(sock);
    return 0;
}
