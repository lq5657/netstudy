#include <unistd.h>
#include <errno.h>
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

ssize_t readn(int fd, void *buf, size_t count)
{
	size_t nleft = count;
	ssize_t nread;
	char* pbuf = (char*)buf;
	while(nleft > 0) {
		if((nread = read(fd, pbuf, nleft)) == -1) {
			if (errno == EINTR) {
				continue;
			}
			return -1;
		} else if (nread == 0) {
			return count - nleft;
		}

		nleft -= nread;
		pbuf += nread;
	}

	return count;
}

ssize_t writen(int fd, const void *buf, size_t count)
{
	size_t nleft = count;
	ssize_t nwriten;
	char* pbuf = (char*)buf;
	while(nleft > 0) {
		if((nwriten = write(fd, pbuf, nleft)) == -1 ) {
			if (errno == EINTR) {
				continue;
			}
			return -1;
		}
		else if (nwriten == 0)
			continue;

		nleft -= nwriten;
		pbuf += nwriten;
	}

	return count;
}

ssize_t recv_cache(int sockfd, void *buf, size_t count)
{
	int ret;
	while(1) {
		ret = recv(sockfd, buf, count, MSG_PEEK);
		if (ret == -1 && errno == EINTR)
			continue;
		return ret;
	}
}

ssize_t readline(int sockfd, void *buf, size_t maxline)
{
	int ret;
	int nread = 0;
	int nleft = maxline;
	char *pbuf = (char*)buf;
	while(1){
		if ((ret = recv_cache(sockfd, pbuf, nleft)) == -1)
			return ret;
		else if (ret == 0)
			return ret;
		
		nread = ret;
		int i = 0;
		for (; i < nread; ++i) {
			if (pbuf[i] == '\n') {
				if((ret = readn(sockfd, pbuf, i+1)) != i+1)
					exit(EXIT_FAILURE);
				return ret;
			}
		}

		if (nread > nleft)
			exit(EXIT_FAILURE);
		if ((ret = readn(sockfd, pbuf, nread)) != nread)
			exit(EXIT_FAILURE);
		nleft -= nread;
		pbuf += nread;
	}
	return -1;
}

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
	servaddr.sin_port = htons(9981);

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
	while(fgets(sendBuf, sizeof(sendBuf), stdin) != NULL) {
		int ret;
		if ((ret = writen(sockfd, sendBuf, strlen(sendBuf))) == -1)
			ERR_EXIT("write");

		if ((ret = readline(sockfd, recvBuf, sizeof(recvBuf))) == -1)
			ERR_EXIT("read");
		else if (ret == 0){
			printf("srv(%s:%d) closed", srvstr, srvport);
			break;
		}
		printf("(%s:%d)server:\n", srvstr, srvport);
		fputs(recvBuf, stdout);
		memset(sendBuf, 0, sizeof(sendBuf));
		memset(recvBuf, 0, sizeof(recvBuf));
		fflush(stdout);
		printf("client:");
	}
	close(sockfd);
}
