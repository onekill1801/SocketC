#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ctype.h>
#define MAX 80 
#define PORT 8888
#define SA struct sockaddr 


int main() 
{ 
	int sockfd, connfd, len; 
	struct sockaddr_in servaddr, cli; 

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	bzero(&servaddr, sizeof(servaddr)); 

	servaddr.sin_family = AF_INET; 
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY); 
	servaddr.sin_port = htons(PORT); 
	int y = bind(sockfd, (SA*)&servaddr, sizeof(servaddr));
	int x = listen(sockfd, 5);
	len = sizeof(cli); 
	connfd = accept(sockfd, (SA*)&cli, &len); 
	char buff[MAX]; 

	printf("Client's IP: %s\n", inet_ntoa(cli.sin_addr));
    printf("Client's port: %d\n", ntohs(cli.sin_port));

	read(connfd, buff, sizeof(buff));
	printf("%s\n", buff); 
	bzero(buff, MAX);

	char temp[] = "Hello Client\n";
	strcpy(buff,temp);
	bzero(temp, sizeof(temp));
	write(connfd, buff, sizeof(buff)); 


	close(sockfd); 
} 
