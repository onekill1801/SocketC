#include <netdb.h> 
#include <stdio.h>  //Add fix error 
#include <stdlib.h> 
#include <string.h> 
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h> 
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

    printf("Client's IP: %s\n", inet_ntoa(cli.sin_addr));
    printf("Client's port: %d\n", ntohs(cli.sin_port));
    char buff[MAX]; 
    int n; 
    for (;;) { 
        bzero(buff, MAX); 
        read(connfd, buff, sizeof(buff)); 
        printf("%s", buff); 
 		for (int i = 0; i < sizeof(buff) - 1; i++)
            buff[i] = toupper(buff[i]);

        write(connfd, buff, sizeof(buff)); 
        if (strncmp("~", buff, 1) == 0) { 
            printf("Server Exit...\n"); 
            break; 
        } 
    }
  
    // After chatting close the socket 
    close(sockfd); 
} 