#include "headerFiles.h"

#define PATH "./sock_addr"

int main() {
	int sock, sock2;
	struct sockaddr_un server, client;
	char buf[256];
	char *quit = "\\quit";
	int buf_len;
	int client_len;

	sock = socket(AF_UNIX, SOCK_STREAM, 0);
	if(sock == -1) {
		perror("socket");
		exit(1);
	}

	server.sun_family = AF_UNIX;
	strcpy(server.sun_path, PATH);

	if(bind(sock, (struct sockaddr *)&server, sizeof(server)) == -1) {
		perror("bind");
		exit(1);
	}

	if(listen(sock, 5) < 0 ) {
		perror("listen");
		exit(1);
	}

	if((sock2 = accept(sock, (struct sockaddr *)&client, &client_len)) == -1) {
			perror("accept");
			exit(1);
		}

	while(1) {
		buf_len = recv(sock2, buf, sizeof(buf), 0);
		if(buf_len == -1) {
			perror("recv");
			exit(1);
		}

		if(strcmp(buf, quit) == 0) break;

		printf("%s\n", buf);
	}

	close(sock);
	return 0;
}



