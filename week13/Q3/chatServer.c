#include "headerFiles.h"

#define QUITMSG "quit\n";
#define PORT 8080
#define BUF_SIZE 256
#define N_CLIENT 3

int sd;
int comm_sock[N_CLIENT];

void handler(int signum)
{
    for (int i = 0; i < N_CLIENT; i++)
    {
        if (comm_sock[i] != -1)
        {
            close(comm_sock[i]);
        }
    }
    close(sd);
}

int maxArr(int arr[])
{
    int max = arr[0];
    for (int i = 1; i < N_CLIENT; i++)
    {
        if (arr[i] > max)
            max = arr[i];
    }
    return max;
}

int main()
{
    signal(SIGINT, handler);

    struct sockaddr_in server, client;
    socklen_t clilen;
    char buf[BUF_SIZE];
    fd_set readfds;
    int ret;
    int i;

    sd = socket(AF_INET, SOCK_STREAM, 0);
    if (sd == -1)
    {
        perror("socket");
        exit(1);
    }

    memset(&server, '\0', sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(PORT);

    if (bind(sd, (struct sockaddr *)&server, sizeof(server)) == -1)
    {
        perror("bind");
        exit(1);
    }

    if (listen(sd, N_CLIENT) < 0)
    {
        perror("listen");
        exit(1);
    }

    for (i = 0; i < N_CLIENT; i++)
    {
        if ((comm_sock[i] = accept(sd, (struct sockaddr *)&client, &clilen)) == -1)
        {
            perror("accpet");
            exit(1);
        }
        else
        {
            printf("Client #%d connected.\n", i);
        }
    }

    while (1)
    {
        FD_ZERO(&readfds);
        for (i = 0; i < N_CLIENT; i++)
        {
            FD_SET(comm_sock[i], &readfds);
        }
        printf("waiting at select ...\n");
        ret = select(maxArr(comm_sock) + 1, &readfds, NULL, NULL, NULL);
        printf("select returned: %d\n", ret);
        switch (ret)
        {
        case -1:
            perror("select");
            exit(1);
        case 0:
            printf("select returns : 0\n");
            break;
        default:
            i = 0;
            while (ret > 0)
            {
                if (FD_ISSET(comm_sock[i], &readfds))
                {
                    memset(buf, 0, sizeof(buf));
                    if (recv(comm_sock[i], buf, sizeof(buf), 0) == -1)
                    {
                        perror("recv");
                        exit(1);
                    }
                    ret--;
                    printf("MSG from client %d : %s\n", i, buf);
                }
                i++;
            }
            break;
        }
    }

    for (i = 0; i < N_CLIENT; i++)
    {
        if (comm_sock[i] != -1)
        {
            close(comm_sock[i]);
        }
    }
    close(sd);
    return 0;
}