#include "headerFiles.h"

#define QUIT "quit\n"
#define BUF_SIZE 512
#define PORT 8080
int unc_sd, uns_sd, inet_sd; // unixServerSD, unixClientSD, inetClientSD
void set_nonblocking(int sock);
void handler(int signum);
char PATH[32];

int main()
    {
        
        char buf[BUF_SIZE];
        socklen_t slen;
        uns_sd = socket(AF_UNIX, SOCK_STREAM, 0);

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

        // connection setup as unix server..
        if (uns_sd == -1)
        {
            perror("socket(unix server)");
            close(unc_sd);
            close(uns_sd);
            close(inet_sd);
            exit(1);
        }

        struct sockaddr_un us;
        struct sockaddr_in i_addr;

        memset(&us, 0, sizeof(us));
        us.sun_family = AF_UNIX;
        strcpy(us.sun_path, PATH);

        if (bind(uns_sd, (struct sockaddr *)&us, sizeof(us)) == -1)
        {
            perror("bind(unix server)");
            close(unc_sd);
            close(uns_sd);
            close(inet_sd);
            exit(1);
        }

        if (listen(uns_sd, 5) < 0)
        {
            perror("listen(unix server)");
            close(unc_sd);
            close(uns_sd);
            close(inet_sd);
            exit(1);
        }

        if ((unc_sd = accept(uns_sd, NULL, &slen)) == -1)
        {
            perror("accpet(unix)");
            close(unc_sd);
            close(uns_sd);
            close(inet_sd);
            exit(1);
        }
        set_nonblocking(unc_sd);

        // connection setup as inet client...
        inet_sd = socket(AF_INET, SOCK_STREAM, 0);
        if (inet_sd == -1)
        {
            perror("socket(inet client)");
            close(unc_sd);
            close(uns_sd);
            close(inet_sd);
            exit(1);
        }

        memset(&i_addr, '\0', sizeof(i_addr));
        i_addr.sin_family = AF_INET;
        i_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        i_addr.sin_port = htons(PORT);

        if (connect(inet_sd, (struct sockaddr *)&i_addr, sizeof(i_addr)) == -1)
        {
            perror("connect(inet client)");
            close(unc_sd);
            close(uns_sd);
            close(inet_sd);
            exit(1);
        }

        set_nonblocking(inet_sd);

        while (1)
        {
            int buflen = recv(inet_sd, buf, sizeof(buf), 0);
            if(buflen < 0) {
                if (errno != EWOULDBLOCK && errno != EAGAIN)
                {
                    perror("recv(inet)");
                    close(unc_sd);
                    close(uns_sd);
                    close(inet_sd);
                    unlink(PATH);
                    exit(1);
                }
            }
            if (buflen > 0)
            {
                if(strcmp(buf, QUIT) == 0) {
                    close(unc_sd);
                    close(uns_sd);
                    close(inet_sd);
                    unlink(PATH);
                    return 0;
                }
                printf("%s", buf);
            }

            memset(buf, 0, sizeof(buf));
            recv(unc_sd, buf, sizeof(buf), 0);
            if (strlen(buf) > 0)
            {
                printf("[Me] %s", buf);

                if (send(inet_sd, buf, sizeof(buf), 0) == -1)
                {
                    perror("send");
                    close(unc_sd);
                    close(uns_sd);
                    close(inet_sd);
                    unlink(PATH);
                    exit(1);
                }
                memset(buf, 0, sizeof(buf));
            }
        }

        close(unc_sd);
        close(uns_sd);
        close(inet_sd);
        unlink(PATH);
        return 0;
}

void set_nonblocking(int sock)
{
    if (fcntl(sock, F_SETFL, fcntl(sock, F_GETFL, 0) | O_NONBLOCK) == -1)
    {
        perror("fnctl");
        close(unc_sd);
        close(uns_sd);
        close(inet_sd);
        unlink(PATH);
        exit(1);
    }
}

void handler(int signum)
{
    close(unc_sd);
    close(uns_sd);
    close(inet_sd);
    unlink(PATH);
    exit(EXIT_SUCCESS);
}