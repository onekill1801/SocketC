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
    int sockfd, connfd; 
	struct sockaddr_in servaddr, cli; 
	char IPaddress[15]; //="127.0.0.1"

	printf("Enter server ipaddress: ");
	fgets(IPaddress, 15, stdin);

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET; 
	servaddr.sin_addr.s_addr = inet_addr(IPaddress); 
	servaddr.sin_port = htons(PORT); 
	int x = connect(sockfd, (SA*)&servaddr, sizeof(servaddr));
  
    char buff[MAX]; 
    int n; 
    for (;;) { 
        bzero(buff, sizeof(buff)); 
        printf("Send to server(~ to exit): "); 
        n = 0; 
        while ((buff[n++] = getchar()) != '\n') 
            ; 
        write(sockfd, buff, sizeof(buff)); 
        bzero(buff, sizeof(buff)); 
        read(sockfd, buff, sizeof(buff)); 
        printf("Server say: %s", buff); 
        if ((strncmp(buff, "~", 1)) == 0) { 
            printf("Client Exit...\n"); 
            break; 
        } 
    } 
  
    close(sockfd); 
} 
