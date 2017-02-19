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
	int len;
	struct package_t sendBuf;
	struct package_t recvBuf;
	memset(&sendBuf, 0, sizeof(sendBuf));
	memset(&recvBuf, 0, sizeof(recvBuf));
	int size = sizeof(sendBuf.len);
	while(fgets(sendBuf.buf, sizeof(sendBuf.buf), stdin) != NULL) {
		int ret;
		len = strlen(sendBuf.buf);
		sendBuf.len = htonl(len);
		if ((ret = writen(sockfd, &sendBuf, size+len)) == -1)
			ERR_EXIT("write");

		if ((ret = readn(sockfd, &recvBuf.len, size)) == -1)
			ERR_EXIT("read");
		else if (ret < size){
			printf("srv(%s:%d) closed", srvstr, srvport);
			break;
		}
		len = ntohl(recvBuf.len);
		if ((ret = readn(sockfd, recvBuf.buf, len)) == -1)
			ERR_EXIT("read");
		else if (ret < len) {
			printf("srv(%s:%d) closed", srvstr, srvport);
			break;
		}
		printf("(%s:%d)server:\n", srvstr, srvport);
		fputs(recvBuf.buf, stdout);
		memset(&sendBuf, 0, sizeof(sendBuf));
		memset(&recvBuf, 0, sizeof(recvBuf));
		fflush(stdout);
		printf("client:");
	}
	close(sockfd);
}
