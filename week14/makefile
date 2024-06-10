# Makefile

# 컴파일러와 플래그 설정
CC = gcc

# 디렉토리 설정
SERVERDIR = server
CLIENTDIR = client

# 서버 소스 파일과 실행 파일
SERVER_SRC = $(SERVERDIR)/server.c
SERVER_OUT = server.out

# 클라이언트 소스 파일과 실행 파일
CLIENT_TERM_SRC = $(CLIENTDIR)/client_term.c
CLIENT_DISP_SRC = $(CLIENTDIR)/client_disp.c

# 클라이언트 실행 파일 접두어 설정
CLIENT_OUT_PREFIXES = c1 c2 c3 c4

# 클라이언트 터미널과 디스플레이 실행 파일 설정
CLIENT_TERM_OUTS = $(addsuffix -term.out, $(CLIENT_OUT_PREFIXES))
CLIENT_DISP_OUTS = $(addsuffix -disp.out, $(CLIENT_OUT_PREFIXES))

# 기본 타겟 설정
all: $(SERVER_OUT) $(CLIENT_TERM_OUTS) $(CLIENT_DISP_OUTS)

# 서버 실행 파일 생성 규칙
$(SERVER_OUT): $(SERVER_SRC)
	$(CC) -pthread -o $@ $^

# 클라이언트 터미널 실행 파일 생성 규칙
c1-term.out: $(CLIENT_TERM_SRC)
	$(CC) -Dc1 -o $@ $^

c2-term.out: $(CLIENT_TERM_SRC)
	$(CC) -Dc2 -o $@ $^

c3-term.out: $(CLIENT_TERM_SRC)
	$(CC) -Dc3 -o $@ $^

c4-term.out: $(CLIENT_TERM_SRC)
	$(CC) -Dc4 -o $@ $^

# 클라이언트 디스플레이 실행 파일 생성 규칙
c1-disp.out: $(CLIENT_DISP_SRC)
	$(CC) -Dc1 -o $@ $^

c2-disp.out: $(CLIENT_DISP_SRC)
	$(CC) -Dc2 -o $@ $^

c3-disp.out: $(CLIENT_DISP_SRC)
	$(CC) -Dc3 -o $@ $^

c4-disp.out: $(CLIENT_DISP_SRC)
	$(CC) -Dc4 -o $@ $^

# 청소 명령
clean:
	rm -f $(SERVER_OUT) $(CLIENT_TERM_OUTS) $(CLIENT_DISP_OUTS)

.PHONY: all clean
