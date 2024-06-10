#include "headerFiles.h"

#define QUIT "quit"
#define BUF_SIZE 512
#define PORT 8080
#define ONBOARDING_MSG "<MENU>\n1.채팅방 목록 보기\n2.채팅방 참여하기(사용법: 2 <채팅창 번호>)\n3.프로그램 종료\n(0을 입력하면 메뉴가 다시 표시됩니다.)\n"

#define ROOM_COUNT 3
#define MAX_USER_EACH_ROOM 5
#define MAX_USER_COUNT ROOM_COUNT * MAX_USER_EACH_ROOM

void set_nonblocking(int sock);
void handler(int signum);
void clear_resources(void);
void* room_th(void* arg);
int getMax(fd_set* arg);

typedef struct client {
    int sd;
} client;

typedef struct room {
    fd_set room_fds;
    client *returned_user;
    client *new_user;
    int client_count;
    int roomNum;
} room;

int sd;
int total_count = 0;
room chatrooms[ROOM_COUNT];
int maxFD = 0;
fd_set main_fds;

struct timeval tv = {10, 0};

int main() {
    int ret;
    char buf[BUF_SIZE];
    socklen_t clen;
    signal(SIGINT, handler);

    // init sd..
    sd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_addr, client_addr;

    if(sd == -1) {
        perror("socket(inet server)");
        clear_resources();
        exit(1);
    }

    // 룸 생성..
    for (int i = 0; i < ROOM_COUNT; i++) {
        chatrooms[i].client_count = 0;
        chatrooms[i].new_user = NULL;
        chatrooms[i].returned_user = NULL;
        chatrooms[i].roomNum = i;
        FD_ZERO(&chatrooms[i].room_fds);

        pthread_t tid;
        if(pthread_create(&tid, NULL, room_th,(void *)&chatrooms[i]) != 0) {
            perror("pthread");
            clear_resources();
            exit(1);
        }
    }

    // 소켓 주소 설정 및 바인딩..
    memset(&server_addr, '\0', sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(PORT);

    if(bind(sd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind(inet server)");
        clear_resources();
        exit(1);
    }

    if(listen(sd, MAX_USER_COUNT) < 0) {
        perror("liseten");
        clear_resources();
        exit(1);
    }

    set_nonblocking(sd);
    maxFD = sd;
    FD_ZERO(&main_fds);
    while (1)
    {   
        // 1. nonblocking accept
        int new_sd = accept(sd, (struct sockaddr *)&client_addr, &clen);
        if(new_sd > 0) {
            set_nonblocking(new_sd);
            printf("[MAIN] 새로운 사용자가 접속했습니다: %d\n", new_sd);
            FD_SET(new_sd, &main_fds);

            send(new_sd, ONBOARDING_MSG, strlen(ONBOARDING_MSG), 0);
            if(sd > maxFD) {
                maxFD = sd;
            }
        }

        // 2. 채팅방에서 탈퇴한 사용자가 있나 확인 후 넣기
        for (int i = 0; i < ROOM_COUNT; i++)
        {
            if(chatrooms[i].returned_user != NULL) {
                printf("[MAIN] 사용자 %d가 %d번 방에서 나왔습니다.\n", chatrooms[i].returned_user->sd, i);
                if (chatrooms[i].returned_user->sd > maxFD)
                    maxFD = chatrooms[i].returned_user->sd;
                FD_SET(chatrooms[i].returned_user->sd, &main_fds);
                chatrooms[i].returned_user = NULL;
            }
        }

        // 3. 대기실의 사용자들을 대상으로, 일정 시간 후에 timeOut하는 select
        ret = select(maxFD + 1, &main_fds, NULL, NULL, &tv);
        if(ret == -1) {
            perror("select");
        } else if(ret == 0) {
            printf("select returns 0\n");
        } else {
            for (int i = 0; i <= maxFD; i++) {
                if(FD_ISSET(i, &main_fds)) {
                    if(recv(i, buf, sizeof(buf), 0) == -1) {
                        perror("recv");
                        clear_resources();
                        exit(1);
                    } else {
                        // 응답 처리..
                        if(strcmp(buf, "1") == 0) {
                            snprintf(buf, sizeof(buf), "<ChatRoom info> \n");
                            for (int j = 0; j < ROOM_COUNT; j++) {
                                char roomInfo[64];
                                snprintf(roomInfo, sizeof(roomInfo), "[ID: %d] Chatroom-%d (%d/%d)\n", j, j, chatrooms[j].client_count, MAX_USER_EACH_ROOM);
                                strncat(buf, roomInfo, sizeof(buf) - sizeof(roomInfo) - 1);
                            }
                            send(i, buf, sizeof(buf), 0);
                        }
                        else if (buf[0] == '2')
                        {
                            int room_num = atoi(&buf[2]);
                            
                            // 방이 꽉차면..
                            if(chatrooms[room_num].client_count >= MAX_USER_EACH_ROOM) {
                                strcpy(buf, "방이 꽉찼습니다.\n");
                                send(i, buf, sizeof(buf), 0);
                            } else {
                                client newUser = {sd};
                                chatrooms[room_num].client_count++;
                                chatrooms[room_num].new_user = &newUser;
                                printf("사용자 %d가 채팅방 %d에 접속합니다.\n", i, room_num);
                                continue;
                            }
                        }
                        else if (strcmp(buf, "3") == 0)
                        {
                            printf("[MAIN] %d사용자와의 접속을 해제합니다.\n", i);
                            close(i);
                        }
                        
                        // 0을 포함하는 이외의 값..
                        send(i, ONBOARDING_MSG, strlen(ONBOARDING_MSG), 0);
                    }
                }
            }
        }
    }

    clear_resources();
    return 0;
}

void* room_th(void* arg) {
    room* MyRoom = (room *) arg;
    char buf[BUF_SIZE];
    while (1)
    {
        // 새 사용자가 있으면 받아
        if(MyRoom -> new_user != NULL) {
            FD_SET(MyRoom -> new_user -> sd, &(MyRoom -> room_fds));
            MyRoom -> new_user = NULL;
        }

        int ret = select(maxFD + 1, &(MyRoom->room_fds), NULL, NULL, NULL);
        switch (ret)
        {
        case -1:
            perror("select");
            break;
        case 0:
            printf("select None\n");
            break;
        default:
            for (int i = 0; i <= maxFD; i++) {
                if(FD_ISSET(i, &(MyRoom -> room_fds))) {
                    int blen = recv(i, buf, sizeof(buf), 0);
                    if(blen < 0) {
                        perror("recv(chatroom)");
                        clear_resources();
                        exit(1);
                    }

                    // quit받으면 나가기
                    if(strcmp(buf, QUIT) == 0) {
                        FD_CLR(i, &(MyRoom->room_fds));
                        MyRoom->returned_user->sd = i;
                        MyRoom->client_count--;
                        continue;
                    }

                    // 혼자있으면 채팅 x
                    if(MyRoom -> client_count == 1) {
                        printf("[CH.%d] 사용자가 혼자여서 메시지를 전달안합니다.\n", MyRoom->roomNum);
                        continue;
                    }

                    // 나머지 유저에게 메시지 전달 + 로그
                    printf("[MAIN] 사용자 %d (방%d) 메시지: %s\n", i, MyRoom->roomNum, buf);
                    for (int j = 0; j <= maxFD; j++)
                    {
                        if(i != j && FD_ISSET(j, &(MyRoom ->room_fds))) {
                            send(j, buf, sizeof(buf), 0);
                        }
                    }
                }
            }
            break;
        }
    }

    return NULL;
}

void clear_resources() {
    for (int i = 0; i <= maxFD; i++) {
        close(i);
    }
    close(sd);
}

void set_nonblocking(int sock)
{
    if (fcntl(sock, F_SETFL, fcntl(sock, F_GETFL, 0) | O_NONBLOCK) == -1)
    {
        perror("fnctl");
        clear_resources();
        exit(1);
    }
}

void handler(int signum)
{
    printf("(시그널 핸들러) 마무리 작업 시작!\n");
    clear_resources();
    exit(EXIT_SUCCESS);
}