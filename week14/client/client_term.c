#include "headerFiles.h"

#define QUIT "quit"
#define BUF_SIZE 512

char PATH[32];
int sd; // socket discripter
void handler(int signum);

int main() {
    char buf[BUF_SIZE];

    signal(SIGINT, handler);

#ifdef c1
    strcpy(PATH, "./c1");
#elif defined(c2)
    strcpy(PATH, "./c2");
#elif defined(c3)
    strcpy(PATH, "./c3");
#elif defined(c4)
    strcpy(PATH, "./c4");
#else
    strcpy(PATH, "./socket");
#endif

    sd = socket(AF_UNIX, SOCK_STREAM, 0);
    if(sd == -1) {
        perror("socket");
        close(sd);
        exit(1);
    }

    struct sockaddr_un addr;
    memset(&addr, '\0', sizeof(addr));
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, PATH);

    if(connect(sd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        perror("connect");
        close(sd);
        exit(1);
    }

    while(1) {
        memset(buf, '\0', sizeof(buf));
        fgets(buf, sizeof(buf), stdin);

        if(send(sd, buf, sizeof(buf), 0) == -1) {
            perror("send");
            close(sd);
            exit(1);
        }
    }
    close(sd);
    return 0;
}

void handler(int signum)
{
    close(sd);
    exit(EXIT_SUCCESS);
}