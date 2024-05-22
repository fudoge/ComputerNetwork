#include "headerFiles.h"

#define PATH "./sock_addr"
#define BUFFER_SIZE 256
char *QUIT = "quit\n";

int main()
{
    int sock;
    struct sockaddr_un addr;
    char buf[BUFFER_SIZE];

    sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock == -1)
    {
        perror("socket");
    }

    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, PATH);

    if (connect(sock, (const struct sockaddr *)&addr, sizeof(addr)) == -1)
    {
        perror("connect");
        exit(1);
    }
    printf("[Info] Unix socket : connected to the server\n");

    while (1)
    {
        printf("ENTER: ");
        if (fgets(buf, sizeof(buf), stdin) != NULL)
        {
            int buflen = send(sock, buf, sizeof(buf), 0);
            if (buflen < 0)
            {
                perror("send");
                exit(1);
            }

            if (strcmp(buf, QUIT) == 0)
            {
                printf("Terminate...\n");
                printf(": Success\n");
                break;
            }
        }
    }
    printf("[Info] Closing socket");
    close(sock);
    return 0;
}