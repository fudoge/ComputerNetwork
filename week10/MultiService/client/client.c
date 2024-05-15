#include "headerFiles.h"

int main(void) {
    int sock;
    struct sockaddr_in addr;
    char buf_recv[1024];
    char buf_send[1024];
    size_t buf_len = 0;
    int option;
    FILE* f;
    const char *quit = "\\quit";
    const char *eof = "\\EOF";

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock == -1) {
        perror("sock");
        exit(1);
    }

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(8000);

    if(connect(sock, (const struct sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("connect");
        exit(1);
    }

    while(1) {

        buf_len = recv(sock, buf_recv, sizeof(buf_recv), 0);
        if(buf_len == -1) {
            perror("recv");
            exit(1);
        }
        printf("%s", buf_recv);

        scanf("%d", &option);
        if(option == 1) {
            strcpy(buf_send, "\\service 1");
            send(sock, buf_send, sizeof(buf_send), 0);

            buf_len = recv(sock, buf_recv, sizeof(buf_recv), 0);
            if (buf_len == -1)
            {
                perror("recv");
                exit(1);
            }
            printf("Server Time: %s", buf_recv);
            continue;
        } else if(option == 2) {
            strcpy(buf_send, "\\service 2");
            send(sock, buf_send, sizeof(buf_send), 0);

            buf_len = recv(sock, buf_recv, sizeof(buf_recv), 0);
            if (buf_len == -1)
            {
                perror("recv");
                exit(1);
            }
            printf("%s", buf_recv);

            scanf("%d", &option);
            sprintf(buf_send, "%d", option);
            send(sock, buf_send, sizeof(buf_send), 0);
            if (option == 1)
            {
                f = fopen("./Book.txt", "wb");
                while (1)
                {
                    buf_len = recv(sock, buf_recv, sizeof(buf_recv), 0);
                    if(buf_len == -1) {
                        perror("recv");
                        exit(1);
                    }
                    if (memcmp(buf_recv, eof, strlen(eof)) == 0)
                        break;
                    if(buf_len > fwrite(buf_recv, 1, buf_len, f)) {
                        perror("fwrite");
                        exit(1);
                    }
                }

                fclose(f);
                continue;
            } else if(option == 2) {
                f = fopen("./HallymUniv.jpg", "wb");
                while (1)
                {
                    buf_len = recv(sock, buf_recv, sizeof(buf_recv), 0);
                    if (buf_len == -1)
                    {
                        perror("recv");
                        exit(1);
                    }
                    if (memcmp(buf_recv, eof, strlen(eof)) == 0)
                        break;
                    if (buf_len > fwrite(buf_recv, 1, buf_len, f))
                    {
                        perror("fwrite");
                        exit(1);
                    }
                }

                fclose(f);
                continue;
            }
            else if (option == 3)
            {
                strcpy(buf_send, "3");
                send(sock, buf_send, sizeof(buf_send), 0);
                continue;
            }
        } else if(option == 3) {
            strcpy(buf_send, "\\service 3");
            send(sock, buf_send, sizeof(buf_send), 0);

            while(1) {
                scanf("%s", buf_send);
                if(send(sock, buf_send, sizeof(buf_send), 0) == -1) {
                    perror("send");
                    exit(1);
                }
                if (strcmp(buf_send, quit) == 0)
                {
                    break;
                }

                buf_len = recv(sock, buf_recv, sizeof(buf_recv), 0);
                if (buf_len == -1)
                {
                    perror("recv");
                    exit(1);
                }

                printf("[You] %s\n", buf_recv);
            }
            continue;
        }
        else
        {
            exit(1);
        }
    }
    
    close(sock);
    return 0;
}