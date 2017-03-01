#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define ERR_EXIT(m) \
	do { \
		perror(m); \
		exit(EXIT_FAILURE); \
	} while(0) \

int main()
{
	int listenfd;
	if ((listenfd = socket(PF_INET, SOCK_STREAM,IPPROTO_TCP)) < 0)
		ERR_EXIT("socket");
	int reuseOn = 1;
	if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &reuseOn, sizeof(reuseOn)) == -1)
		ERR_EXIT("setsockopt");
	struct sockaddr_in servaddr;
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	/*servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");*/
	/*inet_aton("127.0.0.1", &servaddr.sin_addr);*/
	servaddr.sin_port = htons(8818);
	if (bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0)
		ERR_EXIT("bind");
	if (listen(listenfd, SOMAXCONN) < 0)
		ERR_EXIT("listen");

	int connfd;
	struct sockaddr_in cliaddr;
	socklen_t cliaddrLen = sizeof(cliaddr);
	if((connfd = accept(listenfd, (struct sockaddr*)&cliaddr, &cliaddrLen)) < 0)
		ERR_EXIT("accept");
	char clistr[10] = {0};
	int cliport = 0;
	char recvBuf[1024];
	char *paddr = inet_ntoa(cliaddr.sin_addr);
	printf("%s,%lu\n", paddr,strlen(paddr));
	if(memcpy(clistr, paddr, strlen(paddr)) == NULL)
		ERR_EXIT("memcpy");
	cliport = ntohs(cliaddr.sin_port);
	printf("client %s:%d connected\n", clistr, cliport);
	while(1) {
		memset(recvBuf, 0, sizeof(recvBuf));
		int ret;
		if ((ret = read(connfd, recvBuf, sizeof(recvBuf))) == -1)
			ERR_EXIT("read");
		else if (ret == 0)
			break;
		printf("(%s:%d)client:\n", clistr, cliport);
		fflush(stdout);
		fputs(recvBuf, stdout);
		if ((ret = write(connfd, recvBuf, strlen(recvBuf))) == -1)
			ERR_EXIT("write");
	}
	close(connfd);
	close(listenfd);
}
