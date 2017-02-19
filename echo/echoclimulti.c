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
	int sockfd;
	if ((sockfd = socket(PF_INET, SOCK_STREAM,IPPROTO_TCP)) < 0)
		ERR_EXIT("socket");
	struct sockaddr_in servaddr;
	servaddr.sin_family = AF_INET;
	/*servaddr.sin_addr.s_addr = htonl(INADDR_ANY);*/
	servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	/*inet_aton("127.0.0.1", &servaddr.sin_addr);*/
	servaddr.sin_port = htons(8818);

	if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) == -1)
		ERR_EXIT("connect");
	char srvstr[10] = {0};
	int srvport = 0;
	char *paddr = inet_ntoa(servaddr.sin_addr);
	if(memcpy(srvstr, paddr, strlen(paddr)) == NULL)
		ERR_EXIT("memcpy");
	srvport = ntohs(servaddr.sin_port);
	printf("have connected to server %s:%d\n", srvstr, srvport);
	char sendBuf[1024] = {0};
	char recvBuf[1024] = {0};
	printf("client:");
	while(fgets(sendBuf, sizeof(sendBuf), stdin) != NULL) {
		int ret;
		if ((ret = write(sockfd, sendBuf, strlen(sendBuf))) == -1)
			ERR_EXIT("write");
		if ((ret = read(sockfd, recvBuf, sizeof(recvBuf))) == -1)
			ERR_EXIT("read");
		printf("(%s:%d)server:\n", srvstr, srvport);
		fputs(recvBuf, stdout);
		memset(sendBuf, 0, sizeof(sendBuf));
		memset(recvBuf, 0, sizeof(recvBuf));
		fflush(stdout);
		printf("client:");
	}

}
