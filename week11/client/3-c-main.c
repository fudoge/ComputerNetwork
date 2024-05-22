#include "headerFiles.h"
#define BUFFER_SIZE 256
#define PATH "./sock_addr"
#define PORT 8080

char* QUIT = "quit";
int server_socket_fd;
short connection_flag = 1;

void set_nonblocking(int sock)
{
    if (fcntl(sock, F_SETFL, fcntl(sock, F_GETFL, 0) | O_NONBLOCK) == -1)
    {
        perror("fnctl");
        exit(1);
    }
}

void *unix_communication(void *arg)
{
    int unix_server, unix_client;
    struct sockaddr_un u_addr, u_addr_client;
    char buffer[BUFFER_SIZE];
    socklen_t unix_clientlen;

    unix_server = socket(AF_UNIX, SOCK_STREAM, 0);
    if (unix_server < 0)
    {
        perror("sock(unix)");
        exit(1);
    }

    u_addr.sun_family = AF_UNIX;
    strcpy(u_addr.sun_path, PATH);

    if (bind(unix_server, (const struct sockaddr *)&u_addr, sizeof(u_addr)) == -1)
    {
        perror("bind(unix)");
        exit(1);
    }

    if (listen(unix_server, 5) < 0)
    {
        perror("listen(unix)");
        exit(1);
    }
    printf("[Info] Unix socket : waiting for conn req\n");

    unix_client = accept(unix_server, NULL, NULL);
    if (unix_client < 0)
    {
        perror("accpet(unix)");
        exit(1);
    }
    printf("[Info] Unix socket : client connected\n");

    connection_flag = 0;
    set_nonblocking(unix_client);
    set_nonblocking(unix_server);
    while (1)
    {
        int buflen = recv(unix_client, buffer, sizeof(buffer), 0);
        if (buflen < 0)
        {
            if (errno != EWOULDBLOCK && errno != EAGAIN)
            {
                perror("read(unix)");
                exit(1);
            }
        }
        else
        {
            if (send(server_socket_fd, buffer, buflen, 0) < 0)
            {
                perror("send");
                exit(1);
            }
            printf("[Me] %s", buffer);
            if (strcmp(buffer, QUIT) == 0)
            {
                printf("[SERVER] %s\n", buffer);
                printf("[Info] Closing sockets");
                exit(1);
            }
        }
    }

    close(unix_client);
    close(unix_server);
    return NULL;
}

int main() {
    struct sockaddr_in i_addr_server;
    char buf[256];
    pthread_t u_thread;

    if(pthread_create(&u_thread, NULL, unix_communication, NULL) != 0) {
        perror("unix_thread");
        exit(1);
    }

    server_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(server_socket_fd < 0) {
        perror("socket(inet)");
        exit(1);
    }

    i_addr_server.sin_family = AF_INET;
    i_addr_server.sin_addr.s_addr = htonl(INADDR_ANY);
    i_addr_server.sin_port = htons(PORT);

    while(connection_flag) {}

    if(connect(server_socket_fd, (const struct sockaddr*)&i_addr_server, sizeof(i_addr_server)) < 0) {
        perror("connect(inet)");
        exit(1);
    }
    printf("[Info] Inet socket : connected to the server\n");

    set_nonblocking(server_socket_fd);

    while (1)
    {
        memset(buf, 0, BUFFER_SIZE);
        int buflen = recv(server_socket_fd, buf, sizeof(buf), 0);
        if(buflen < 0) {
            if (errno != EWOULDBLOCK && errno != EAGAIN)
            {
                perror("read(inet)");
                exit(1);
            }
        } else if(buflen > 0) {
            printf("[YOU] %s", buf);
        }
    }

    close(server_socket_fd);
    return 0;
}