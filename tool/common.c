
#define ERR_EXIT(m) \
	do { \
		perror(m); \
		exit(EXIT_FAILURE); \
	} while(0) \

struct package_t {
	int len;
	char buf[1024];
};

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
