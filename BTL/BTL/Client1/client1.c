#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <time.h> 

#define MAX_LINE 4096
#define LINSTENPORT 7788
#define SERVERPORT 8877
#define BUFFSIZE 4096

void sendfile(FILE *fp, int sockfd);
void writefile(int sockfd, FILE *fp);
void multiSend(char *filename, char *ipadderssClient);
ssize_t total=0;
int main(int argc, char *argv[]) 
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
    {
        perror("Can't allocate sockfd");
        exit(1);
    }

    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(SERVERPORT);
    if (inet_pton(AF_INET, "192.168.1.2", &serveraddr.sin_addr) < 0)
    {
        perror("IPaddress Convert Error");
        exit(1);
    }
    if (connect(sockfd, (const struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0)
    {
        perror("Connect Error");
        exit(1);
    }

    char filename[BUFFSIZE] = {0}; 
    if (recv(sockfd, filename, BUFFSIZE, 0) == -1) 
    {
        perror("Can't receive filename");
        exit(1);
    }
    printf("filename: %s\n", filename);

    FILE *fp = fopen(filename, "wb");
    if (fp == NULL) 
    {
        perror("Can't open file");
        exit(1);
    }
    
    char addr[INET_ADDRSTRLEN];
    // printf("Start receive file: %s from %s\n", filename, inet_ntop(AF_INET, &clientaddr.sin_addr, addr, INET_ADDRSTRLEN));
    writefile(sockfd, fp);
    printf("Receive Success, NumBytes = %d\n", total);
    fclose(fp);
    // ------------------------------------------------------------------------------------------------

    close(sockfd);
    multiSend(filename,"10.10.2.2");
    multiSend(filename,"10.10.3.2");
    return 0;
}

void writefile(int sockfd, FILE *fp)
{
    ssize_t n;
    char buff[MAX_LINE] = {0};
    while ((n = recv(sockfd, buff, MAX_LINE, 0)) > 0) 
    {
        total+=n;
        if (n == -1)
        {
            perror("Receive File Error");
            exit(1);
        }
        
        if (fwrite(buff, sizeof(char), n, fp) != n)
        {
            perror("Write File Error");
            exit(1);
        }
        memset(buff, 0, MAX_LINE);
    }
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

void multiSend(char *filename, char *ipadderssClient){
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
    {
        perror("Can't allocate sockfd");
        exit(1);
    }

    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(SERVERPORT);
    if (inet_pton(AF_INET, ipadderssClient, &serveraddr.sin_addr) < 0)
    {
        perror("IPaddress Convert Error");
        exit(1);
    }

    if (connect(sockfd, (const struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0)
    {
        perror("Connect Error");
        exit(1);
    }
    
    // char *filename = basename(filename); 
    if (filename == NULL)
    {
        perror("Can't get filename");
        exit(1);
    }
    
    char buff[BUFFSIZE] = {0};
    strncpy(buff, filename, strlen(filename));
    if (send(sockfd, buff, BUFFSIZE, 0) == -1)
    {
        perror("Can't send filename");
        exit(1);
    }
    
    FILE *fp = fopen(filename, "rb");
    if (fp == NULL) 
    {
        perror("Can't open file");
        exit(1);
    }

    sendfile(fp, sockfd);
    puts("Send Success");
    printf("Send Success to %s, NumBytes = %d\n",ipadderssClient, total);
    fclose(fp);
    close(sockfd);
}
















































// #include <sys/socket.h>
// #include <sys/types.h>
// #include <netinet/in.h>
// #include <netdb.h>
// #include <stdio.h>
// #include <string.h>
// #include <stdlib.h>
// #include <unistd.h>
// #include <errno.h>
// #include <arpa/inet.h>

// #define MAX_LINE 4096
// #define BUFFSIZE 4096
// ssize_t total=0;

// void writefile(int sockfd, FILE *fp)
// {
//     ssize_t n;
//     char buff[MAX_LINE] = {0};
//     printf("writefile_breakPoint1\n");

//     while ((n = recv(sockfd, buff, MAX_LINE, 0)) > 0) 
//     {
//         total+=n;
//         if (n == -1)
//         {
//             perror("Receive File Error");
//             exit(1);
//         }
        
//         if (fwrite(buff, sizeof(char), n, fp) != n)
//         {
//             perror("Write File Error");
//             exit(1);
//         }
//         memset(buff, 0, MAX_LINE);
//     }
//     printf("writefile_breakPoint2\n");

// }

// int main(int argc, char *argv[]){

//     int sfd =0, n=0, b;
//     char rbuff[BUFFSIZE];
//     char sendbuffer[100];

//     struct sockaddr_in serv_addr;

//     memset(rbuff, '0', sizeof(rbuff));
//     sfd = socket(AF_INET, SOCK_STREAM, 0);

//     serv_addr.sin_family = AF_INET;
//     serv_addr.sin_port = htons(5000);
//     serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

//     b=connect(sfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
//     char buff[BUFFSIZE]= "Hello Server";
//     write(sfd, buff, sizeof(buff));

//     if (b==-1) {
//         perror("Connect");
//         return 1;
//     }
// 	printf("breakPoint1\n");

//     // FILE *fp = fopen("README.md", "rb");
//     // writefile(sfd,fp);
// 	printf("breakPoint3\n");

//     // fclose(fp);
//     close(sfd);
//     return 0;

// }