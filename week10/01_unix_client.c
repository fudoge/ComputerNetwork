#include "headerFiles.h"

#define PATH "./sock_addr"

int main() {
	int sock;
	struct sockaddr_un addr;
	char buf[256];
	char *esc = "\\quit";

	sock = socket(AF_UNIX, SOCK_STREAM, 0);
	if(sock == -1) {
		perror("socket");
		exit(1);
	}

	addr.sun_family = AF_UNIX;
	strcpy(addr.sun_path, PATH);

	if(connect(sock, (const struct sockaddr	*)&addr, sizeof(addr)) == -1) {
		perror("connect");
		exit(1);
	}

	while(1) {
		scanf("%s", buf);

		if(send(sock, buf, sizeof(buf), 0) == -1) {
			perror("send");
			exit(1);
		}

		if(strcmp(buf, esc) == 0) break;
	}
	
	return 0;
}	
