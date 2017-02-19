#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ERR_EXIT(m) \
	do { \
		perror(m); \
		exit(EXIT_FAILURE); \
	} while(0) \

int getlocalip(char* ip, int n) 
{
    if (ip == NULL || n < 9)
        return -1;
    char hostname[128] = {0};
    if(gethostname(hostname, sizeof(hostname)) == -1)
        return -1;
    
    struct hostent *ht = gethostbyname(hostname);
    if (ht == NULL)
        return -1;
    memcpy(ip, inet_ntoa(*(struct in_addr*)ht->h_addr), 9);
    return 0;
}

int main() 
{
    char hostname[128] = {0};
    if(gethostname(hostname, sizeof(hostname)) == -1)
        ERR_EXIT("gethostname");
    
    struct hostent *ht = gethostbyname(hostname);
    if (ht == NULL)
        ERR_EXIT("gethostbyname");
    int i = 0;
    while(ht->h_addr_list[i] != NULL) {
        printf("%s\n",inet_ntoa(*(struct in_addr*)ht->h_addr_list[i]));
        i++;
    }

    char ip[10] = {0};
    getlocalip(ip, sizeof(ip));
    printf("local ip:%s\n", ip);
}