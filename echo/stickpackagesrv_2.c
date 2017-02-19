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

void do_service(int fd)
{
	char recvBuf[1024] = {0};
	while(1) {
		int ret;
		if((ret = readline(fd, recvBuf, sizeof(recvBuf))) == -1)
			ERR_EXIT("readline");
		else if (ret == 0) {
			printf("client close\n");
			break;
		}

		printf("client:");
		fputs(recvBuf, stdout);

		if((ret = writen(fd, recvBuf, strlen(recvBuf))) == -1)
			ERR_EXIT("writen");
        memset(recvBuf, 0, sizeof(recvBuf));
	}
}

int main()
{
	int listenfd;
	if((listenfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
		ERR_EXIT("socket");
	int reuseOn = 1;
	if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &reuseOn, sizeof(reuseOn)) == -1)
		ERR_EXIT("setsockopt");
	struct sockaddr_in srvaddr;
	srvaddr.sin_family = AF_INET;
	srvaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	srvaddr.sin_port = htons(9981);
	if((bind(listenfd, (struct sockaddr*)&srvaddr, sizeof(srvaddr))) == -1)
		ERR_EXIT("bind");
	if((listen(listenfd, SOMAXCONN)) == -1)
		ERR_EXIT("listen");
	int connfd;
	struct sockaddr_in cliaddr;
	socklen_t clilen = sizeof(cliaddr);
	if((connfd = accept(listenfd, (struct sockaddr*)&cliaddr, &clilen)) == -1)
		ERR_EXIT("accept");
	do_service(connfd);
	close(connfd);
}
