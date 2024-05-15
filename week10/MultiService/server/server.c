#include "headerFiles.h"
#include <time.h>

int main() {
    int s_client, s_server;
    struct sockaddr_in client, server;
    socklen_t client_len;
    int buf_len;
    char buf_recv[1024];
    char buf_send[1024];
    FILE *f;
    const char *quit = "\\quit";
    const char *eof = "\\EOF";

    s_server = socket(AF_INET, SOCK_STREAM, 0);
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(8000);

    if(bind(s_server, (const struct sockaddr*)&server, sizeof(server)) == -1) {
        perror("bind");
        exit(1);
    }

    if(listen(s_server, 5) < 0) {
        perror("listen");
        exit(1);
    }

    if ((s_client = accept(s_server, (struct sockaddr *)&server, &client_len)) == -1)
    {
        perror("accpet");
        exit(1);
    }

    while(1) {
        strcpy(buf_send, "[Service List]\n1. Get Currnet Time\n2. Download File\n3. Echo Server\nEnter: ");
        send(s_client, buf_send, sizeof(buf_send), 0);

        buf_len = recv(s_client, buf_recv, sizeof(buf_recv), 0);
        if(buf_len == -1) {
            perror("recv");
            exit(1);
        }

        if(strcmp(buf_recv, "\\service 1") == 0) {
            time_t rawtime;
            struct tm *timeinfo;

            time(&rawtime);
            timeinfo = localtime(&rawtime);
            strcpy(buf_send, asctime(timeinfo));
            send(s_client, buf_send, sizeof(buf_send), 0);
            continue;
        }
        else if (strcmp(buf_recv, "\\service 2") == 0)
        {
            strcpy(buf_send, "[Available File List]\n1. Book.txt\n2. HallymUniv.jpg\n3. Go back\nEnter: ");
            send(s_client, buf_send, sizeof(buf_send), 0);

            buf_len = recv(s_client, buf_recv, sizeof(buf_recv), 0);
            if (buf_len == -1)
            {
                perror("recv");
                exit(1);
            }

            if (strcmp(buf_recv, "1") == 0)
            {
                size_t bytes_read;
                f = fopen("./Book.txt", "rb");
                while ((bytes_read = fread(buf_send, 1, sizeof(buf_send), f)) > 0)
                {
                    if(send(s_client, buf_send, bytes_read, 0) == -1) {
                        perror("send");
                        exit(1);
                    }
                }
                sleep(1);

                if (send(s_client, eof, strlen(eof), 0) == -1)
                {
                    perror("send");
                    exit(1);
                }
                fclose(f);
                continue;
            }
            else if (strcmp(buf_recv, "2") == 0)
            {
                size_t bytes_read;
                f = fopen("./HallymUniv.jpg", "rb");
                while ((bytes_read = fread(buf_send, 1, sizeof(buf_send), f)) > 0)
                {
                    if (send(s_client, buf_send, bytes_read, 0) == -1)
                    {
                        perror("send");
                        exit(1);
                    }
                }
                sleep(1);

                if (send(s_client, eof, strlen(eof), 0) == -1)
                {
                    perror("send");
                    exit(1);
                }
                fclose(f);
                continue;
            }
            else if (strcmp(buf_recv, "3") == 0)
            {
                continue;
            }
            else
            {
                exit(1);
            }
        }
        else if (strcmp(buf_recv, "\\service 3") == 0)
        {
            while (1)
            {
                buf_len = recv(s_client, buf_recv, sizeof(buf_recv), 0);
                if (buf_len == -1)
                {
                    perror("recv");
                    exit(1);
                }

                if (strcmp(buf_recv, quit) == 0)
                {
                    break;
                }

                strcpy(buf_send, buf_recv);
                send(s_client, buf_send, sizeof(buf_send), 0);
            }
            continue;
        }
    }
    close(s_server);
    close(s_client);
    return 0;
}