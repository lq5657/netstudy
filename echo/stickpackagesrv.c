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

struct package_t {
	int len;
	char buf[1024];
};

void do_service(int fd)
{
	struct package_t recvBuf;
	int size = sizeof(recvBuf.len);
	int len;
	while(1) {
		memset(&recvBuf, 0, sizeof(recvBuf));
		int ret;
		if((ret = readn(fd, &recvBuf.len, size)) == -1)
			ERR_EXIT("readn");
		else if (ret < size) {
			printf("client close\n");
			break;
		}

		len = ntohl(recvBuf.len);
		if((ret = readn(fd, &recvBuf.buf, len)) == -1)
			ERR_EXIT("readn");
		else if (ret < len) {
			printf("client close\n");
			break;
		}
		printf("client:");
		fputs(recvBuf.buf, stdout);

		if((ret = writen(fd, &recvBuf, size+len)) == -1)
			ERR_EXIT("writen");
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
