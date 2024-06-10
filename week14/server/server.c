#include "headerFiles.h"

#define QUIT "quit\n"
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
void add_user(int usd, int arr[], int size);
void delete_user(int usd, int arr[], int size);

typedef struct room {
    int users[MAX_USER_EACH_ROOM];
    int returned_user;
    int client_count;
    int roomNum;
} room;

int sd;
int total_count = 0;
room chatrooms[ROOM_COUNT];
int lobby[MAX_USER_COUNT];

int main() {
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

    // 로비 초기화
    memset(lobby, -1, sizeof(lobby)); 
    // 룸 생성..
    for (int i = 0; i < ROOM_COUNT; i++) {
        chatrooms[i].client_count = 0;
        chatrooms[i].returned_user = -1;
        chatrooms[i].roomNum = i;
        memset(chatrooms[i].users, -1, sizeof(chatrooms[i].users));

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
    
    while (1)
    {
        // 1. nonblocking accept
        int new_sd = accept(sd, (struct sockaddr *)&client_addr, &clen);
        if(new_sd > 0) {
            set_nonblocking(new_sd);
            printf("[MAIN] 새로운 사용자가 접속했습니다: %d\n", new_sd);
            add_user(new_sd, lobby, MAX_USER_COUNT);
            send(new_sd, ONBOARDING_MSG, strlen(ONBOARDING_MSG) + 1, 0);
        }

        // 2. 채팅방에서 탈퇴한 사용자가 있나 확인 후 넣기
        for (int i = 0; i < ROOM_COUNT; i++)
        {
            if(chatrooms[i].returned_user != -1) {
                add_user(chatrooms[i].returned_user, lobby, MAX_USER_COUNT);
                send(chatrooms[i].returned_user, ONBOARDING_MSG, strlen(ONBOARDING_MSG) + 1, 0);
                chatrooms[i].returned_user = -1;
            }
        }

        // 3. 대기실의 사용자
            for (int i = 0; i < MAX_USER_COUNT; i++) {
                if(lobby[i] != -1) {
                    int buflen = recv(lobby[i], buf, sizeof(buf), 0);
                    if (buflen < 0)
                    {
                        if(errno != EWOULDBLOCK &&errno != EAGAIN) {
                            perror("recv");
                            clear_resources();
                            exit(1);
                        }
                    }
                    else if(buflen > 0)
                    {
                        buf[buflen] = '\0';
                        // 응답 처리..
                        if (strcmp(buf, "1\n") == 0)
                        {
                            snprintf(buf, sizeof(buf), "<ChatRoom info> \n");
                            for (int j = 0; j < ROOM_COUNT; j++) {
                                char roomInfo[64];
                                snprintf(roomInfo, sizeof(roomInfo), "[ID: %d] Chatroom-%d (%d/%d)\n", j, j, chatrooms[j].client_count, MAX_USER_EACH_ROOM);
                                strncat(buf, roomInfo, sizeof(buf) - sizeof(roomInfo) - 1);
                            }
                            send(lobby[i], buf, sizeof(buf), 0);
                        }
                        else if (buf[0] == '2')
                        {
                            int room_num = atoi(&buf[2]);
                            
                            // 방이 꽉차면..
                            if(chatrooms[room_num].client_count >= MAX_USER_EACH_ROOM) {
                                strcpy(buf, "방이 꽉찼습니다.\n");
                                send(lobby[i], buf, sizeof(buf), 0);
                            } else {
                                // 들어가도 되면..
                                printf("[MAIN] 사용자 %d가 채팅방 %d에 참여합니다.\n", lobby[i], room_num);
                                // 채팅룸에 추가
                                add_user(lobby[i], chatrooms[room_num].users, MAX_USER_EACH_ROOM);
                                chatrooms[room_num].client_count++;
                                // 로비에서 제거
                                delete_user(lobby[i], lobby, MAX_USER_COUNT);
                                continue;
                            }
                        }
                        else if (strcmp(buf, "3\n") == 0)
                        {
                            printf("[MAIN] %d사용자와의 접속을 해제합니다.\n", lobby[i]);
                            send(lobby[i], QUIT, sizeof(QUIT), 0);
                            delete_user(lobby[i], lobby, MAX_USER_COUNT);
                            close(lobby[i]);
                        }
                        
                        // 0을 포함하는 이외의 값..
                        send(lobby[i], ONBOARDING_MSG, strlen(ONBOARDING_MSG)+1, 0);
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
        for (int i = 0; i <= MAX_USER_EACH_ROOM; i++) {
            if(MyRoom->users[i] != -1) {
                int blen = recv(MyRoom -> users[i], buf, sizeof(buf), 0);
                if(blen < 0) {
                    if (errno != EWOULDBLOCK && errno != EAGAIN)
                    {
                        perror("recv");
                        clear_resources();
                        exit(1);
                    }
                }
                if(blen > 0) {
                    // quit받으면 나가기
                    if(strcmp(buf, QUIT) == 0) {
                        printf("[Ch.%d] 사용자 %d를 채팅창에서 제거합니다.\n", MyRoom->roomNum,  MyRoom -> users[i]);
                        MyRoom->returned_user = MyRoom->users[i];
                        delete_user(MyRoom->users[i], MyRoom->users, MAX_USER_EACH_ROOM);
                        MyRoom->client_count--;
                        continue;
                    }
                    // 혼자있으면 채팅 x
                    if(MyRoom -> client_count == 1) {
                        printf("[CH.%d] 사용자가 혼자여서 메시지를 전달안합니다.\n", MyRoom->roomNum);
                        continue;
                    }

                    // 나머지 유저에게 메시지 전달 + 로그
                    printf("[CH.%d] 사용자 %d (방%d) 메시지: %s", MyRoom->roomNum, MyRoom->users[i], MyRoom->roomNum, buf);
                    for (int j = 0; j < MAX_USER_EACH_ROOM; j++)
                    {
                        if (i != j && MyRoom->users[j] != -1)
                        {
                            char message_with_id[BUF_SIZE + 32];
                            snprintf(message_with_id, sizeof(message_with_id), "[%d] %s", MyRoom->users[i], buf);
                            send(MyRoom->users[j], message_with_id, strlen(message_with_id) + 1, 0);
                        }
                    }
                }
            }
        }
    }

    return NULL;
}

void clear_resources() {
    for (int i = 4; i <= MAX_USER_COUNT; i++) {
        close(i);
    }
    close(sd);
}

void add_user(int usd, int arr[], int size) {
    for (int i = 0; i < size; i++) {
        if(arr[i] == -1) {
            arr[i] = usd;
            return;
        }
    }
}

void delete_user(int usd, int arr[], int size) {
    for (int i = 0; i < size; i++) {
        if(arr[i] == usd) {
            arr[i] = -1;
            return;
        }
    }
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