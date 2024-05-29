#include "headerFiles.h"

#define PORT 8080
#define BUF_SIZE 256
#define QUIT "quit\n"

int main()
{
    int sd, csd; // socket descriptor, client socket descriptor
    struct sockaddr_in server, client;
    socklen_t client_len;
    int client_cnt = 0;
    char buf[BUF_SIZE];

    sd = socket(AF_INET, SOCK_STREAM, 0);
    if (sd == -1)
    {
        perror("server socket error");
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

    if (listen(sd, 5) < 0)
    {
        perror("listen");
        exit(1);
    }

    while (1)
    {
        client_len = sizeof(client); // Initialize client_len with size of client structure
        csd = accept(sd, (struct sockaddr *)&client, &client_len);
        if (csd == -1)
        {
            perror("accept");
            continue; // Don't exit, continue to accept other connections
        }
        printf("New Client!\n");
        printf("Number of service clients: %d\n", ++client_cnt);

        switch (fork())
        {
        case 0: // Child process
            close(sd);
            strcpy(buf, "Welcome to Server");
            if (send(csd, buf, strlen(buf) + 1, 0) == -1)
            {
                perror("send");
                exit(1);
            }

            while (1)
            {
                int bytes_received = recv(csd, buf, sizeof(buf), 0);
                if (bytes_received < 0)
                {
                    perror("recv");
                    exit(1);
                }
                // buf[bytes_received] = '\0';

                if (strcmp(buf, QUIT) == 0)
                {
                    printf("Client  quit\n");
                    break;
                }

                printf("Recv from Client: %s\n", buf);

                if (send(csd, buf, bytes_received + 1, 0) == -1)
                {
                    perror("send");
                    exit(1);
                }
            }

            close(csd);
            exit(1);
        default:
            close(csd);
            break;
        case -1:
            perror("fork");
            break;
        }
    }
    close(sd);
    return 0;
}
