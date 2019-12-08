#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#define MAX_LINE 4096
#define LINSTENPORT 7788
#define SERVERPORT 8877
#define BUFFSIZE 4096

void sendfile(FILE *fp, int sockfd);
ssize_t total=0;
int main(int argc, char* argv[])
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) 
    {
        perror("Can't allocate sockfd");
        exit(1);
    }
    
    struct sockaddr_in clientaddr, serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons(SERVERPORT);

    if (bind(sockfd, (const struct sockaddr *) &serveraddr, sizeof(serveraddr)) == -1) 
    {
        perror("Bind Error");
        exit(1);
    }

    if (listen(sockfd, LINSTENPORT) == -1) 
    {
        perror("Listen Error");
        exit(1);
    }

    socklen_t addrlen = sizeof(clientaddr);
    int connfd = accept(sockfd, (struct sockaddr *) &clientaddr, &addrlen);
    if (connfd == -1) 
    {
        perror("Connect Error");
        exit(1);
    }

    char filename[15]; 
    printf("Enter fileName: ");
    scanf("%s",fileName);

    if (filename == NULL)
    {
        perror("Can't get filename");
        exit(1);
    }
    printf("%s", filename);
    char buff[BUFFSIZE] = {0};
    strncpy(buff, filename, strlen(filename));
    if (send(connfd, buff, BUFFSIZE, 0) == -1)
    {
        perror("Can't send filename");
        exit(1);
    }

    // ----------------------------------------------------------------------------------------------
    
    FILE *fp = fopen(filename, "rb");
    if (fp == NULL) 
    {
        perror("Can't open file");
        exit(1);
    }

    sendfile(fp, connfd);
    //puts("Send Success");
    printf("Send Success, NumBytes = %d\n", total);
    fclose(fp);
    // ----------------------------------------------------------------------------------------------

    close(sockfd);
    return 0;
}

void sendfile(FILE *fp, int sockfd) 
{
    int n; 
    char sendline[MAX_LINE] = {0}; 

    while ((n = fread(sendline, sizeof(char), MAX_LINE, fp)) > 0) 
    {
        total+=n;
        if (n != MAX_LINE && ferror(fp))
        {
            perror("Read File Error");
            exit(1);
        }
        
        if (send(sockfd, sendline, n, 0) == -1)
        {
            perror("Can't send file");
            exit(1);
        }
        memset(sendline, 0, MAX_LINE);
    }
}





















































// #include <stdio.h>
// #include <sys/socket.h>
// #include <netinet/in.h>
// #include <arpa/inet.h>
// #include <stdio.h>
// #include <stdlib.h>
// #include <unistd.h>
// #include <errno.h>
// #include <string.h>
// #include <sys/types.h>

// #define MAX_LINE 4096
// #define BUFFSIZE 1024

// ssize_t total=0;
// void sendfile(FILE *fp, int sockfd) 
// {
//     printf("sendfile_breakPoint1\n");
//     int n; 
//     char sendline[MAX_LINE] = {0}; 
//     while ((n = fread(sendline, sizeof(char), MAX_LINE, fp)) > 0) 
//     {
//         total+=n;
//         if (n != MAX_LINE && ferror(fp))
//         {
//             perror("Read File Error");
//             exit(1);
//         }
        
//         if (send(sockfd, sendline, n, 0) == -1)
//         {
//             perror("Can't send file");
//             exit(1);
//         }
//         memset(sendline, 0, MAX_LINE);
//     }
//     printf("sendfile_breakPoint2\n");

// }


// int main(int argc, char *argv[]){

//     int fd =0, confd = 0,b,tot;
//     struct sockaddr_in serv_addr;

//     char buff[BUFFSIZE];
//     int num;

//     fd = socket(AF_INET, SOCK_STREAM, 0);
//     printf("Socket created\n");

//     memset(&serv_addr, '0', sizeof(serv_addr));
//     memset(buff, '0', sizeof(buff));

//     serv_addr.sin_family = AF_INET;
//     serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
//     serv_addr.sin_port = htons(5000);

//     bind(fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
//     listen(fd, 10);

//     printf("breakPoint1\n");
//     confd = accept(fd, (struct sockaddr*)NULL, NULL);
//     read(confd, buff, sizeof(buff));
//     printf("%s\n", buff); 
//     bzero(buff, BUFFSIZE);
//     printf("breakPoint2\n");

//     // FILE* fp = fopen("README.md", "wb");
//     // printf("breakPoint2.1\n");
//     // tot=0;
//     // sendfile(fp,fd);

//     printf("breakPoint3\n");
//     close(confd);

//     return 0;
// }