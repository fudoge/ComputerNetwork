#include "headerFiles.h"

#define PORT 8000

int main() {
    int s_server, s_client;
    struct sockaddr_in server, client;
    int client_len;
    char buf_recv[1000];
    char buf_send[1000];
    char *quit = "\\quit";
    int buf_len;


    s_server = socket(AF_INET, SOCK_STREAM, 0);

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(PORT);

    if(bind(s_server, (struct sockaddr *)&server, sizeof(server)) == -1) {
        perror("bind");
        exit(1);
    }

    if(listen(s_server, 5) < 0) {
        perror("listen");
        exit(1);
    }

    if((s_client = accept(s_server, (struct sockaddr *)&client, &client_len)) == -1) {
            perror("accpet");
            exit(1);
    }

    while(1) {
        buf_len = recv(s_client, buf_recv, sizeof(buf_recv), 0);
        if(buf_len < 0) {
            perror("recv");
            exit(1);
        }

        if(strcmp(buf_recv, quit) == 0) {
            break;
        }

        printf("[You] %s\n", buf_recv);
        scanf("%s", buf_send);

        if(send(s_client, buf_send, sizeof(buf_send), 0) == -1) {
            perror("send");
            exit(1);
        }

        if(strcmp(buf_send, quit) == 0) {
            break;
        }
    }
    close(s_server);
    return 0;
}