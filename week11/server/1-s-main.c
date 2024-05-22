#include "headerFiles.h"

#define PATH "./sock_addr"
#define PORT 8080
#define BUFFER_SIZE 256

char *QUIT = "quit\n";

void set_nonblocking(int sock) {
    if(fcntl(sock, F_SETFL, fcntl(sock, F_GETFL, 0) | O_NONBLOCK) == -1) {
        perror("fnctl");
        exit(1);
    }
}

int main() {
    int server_socket_fd, client_socket_fd;
    struct sockaddr_in i_addr_server, i_addr_client;

    int unix_server, unix_client;
    struct sockaddr_un u_addr;
    char buf[BUFFER_SIZE];
    socklen_t len;

    // unix listening start...
    unix_server = socket(AF_UNIX, SOCK_STREAM, 0);
    if(unix_server < 0) {
        perror("sock(unix)");
        exit(1);
    }

    server_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(server_socket_fd < 0 ) {
        perror("socket(inet)");
        exit(1);
    }

    unlink(PATH);
    u_addr.sun_family = AF_UNIX;
    strcpy(u_addr.sun_path, PATH);

    if(bind(unix_server, (const struct sockaddr*)&u_addr, sizeof(u_addr)) == -1) {
        perror("bind(unix)");
        exit(1);
    }

    if(listen(unix_server, 5) < 0) {
        perror("listen(unix)");
        exit(1);
    }
    printf("[Info] Unix socket : waiting for conn req\n");

    unix_client = accept(unix_server,NULL, NULL);
    if(unix_client < 0) {
        perror("accpet(unix)");
        exit(1);
    }
    printf("[Info] Unix socket : client connected\n");

    set_nonblocking(unix_client);
    set_nonblocking(unix_server);

    // inet server start..
    i_addr_server.sin_family = AF_INET;
    i_addr_server.sin_addr.s_addr = htonl(INADDR_ANY);
    i_addr_server.sin_port = htons(PORT);
    if(bind(server_socket_fd, (const struct sockaddr*)&i_addr_server, sizeof(i_addr_server)) == -1) {
        perror("bind(inet)");
        exit(1);
    }

    if(listen(server_socket_fd, 5) < 0) {
        perror("listen(inet)");
        exit(1);
    }
    printf("[Info] Inet socket : waiting for conn req\n");

    client_socket_fd = accept(server_socket_fd, (struct sockaddr *)&i_addr_client, &len);
    if(client_socket_fd < 0) {
        perror("accpet(inet)");
        exit(1);
    }
    printf("[Info] Inet socket: client connected\n");

    set_nonblocking(client_socket_fd);
    set_nonblocking(server_socket_fd);

    while(1) {
        memset(buf, 0, BUFFER_SIZE);
        int buflen = recv(client_socket_fd, buf, sizeof(buf),0);
        if(buflen < 0) {
            if (errno != EWOULDBLOCK && errno != EAGAIN) {
                perror("read(inet)");
                exit(1);
            }
        } else if(buflen > 0) {
            printf("[YOU] %s", buf);
        }

        buflen = recv(unix_client, buf, sizeof(buf), 0);
        if(buflen < 0) {
            if(errno != EWOULDBLOCK && errno != EAGAIN) {
                perror("read(unix)");
                exit(1);
            }
        } else {
            if (send(client_socket_fd, buf, buflen, 0) < 0)
            {
                perror("writing to server");
                exit(1);
            }
            printf("[Chaewoon Kang] %s", buf);
            if(strcmp(buf, QUIT) == 0) {
                printf("[SERVER] %s\n", buf);
                printf("[Info] Closing sockets");
                exit(1);
            }
        }
    }

    close(client_socket_fd);
    close(server_socket_fd);
    return 0;
}