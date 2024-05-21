#include "headerFiles.h"

#define PATH "./sock_addr"
#define BUFFER_SIZE 256

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

    while (1)
    {
        if (fgets(buf, sizeof(buf), stdin) != NULL)
        {
            int buflen = write(sock, buf, sizeof(buf));
            if (buflen < 0)
            {
                perror("writing");
                exit(1);
            }
        }
    }

    close(sock);
    return 0;
}