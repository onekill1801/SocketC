#include <netdb.h> 
#include <stdio.h>  //Add fix error 
#include <stdlib.h> 
#include <string.h> 
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h> 
#define MAX 80 
#define PORT 8888
#define SA struct sockaddr 

int main() 
{ 
	int sockfd, connfd; 
	struct sockaddr_in servaddr, cli; 
	char IPaddress[15]; //="127.0.0.1"

	printf("Enter server ipaddress: ");
	scanf("%s",IPaddress);

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET; 
	servaddr.sin_addr.s_addr = inet_addr(IPaddress); 
	servaddr.sin_port = htons(PORT); 
	int x = connect(sockfd, (SA*)&servaddr, sizeof(servaddr));
	
	char buff[MAX]= "Hello Server";
	write(sockfd, buff, sizeof(buff));

	bzero(buff, sizeof(buff));
	read(sockfd, buff, sizeof(buff)); 
	printf("%s", buff); 

	close(sockfd); 
} 