/**
 +-------------------------------------------------+
 | Project     : Final Exam                        |
 | Author      : Tung Pham Thanh                   |
 | Student ID  : 17020042                          |
 | Class       : QH - 2017 - I / CQ - C - A - C    |
 +-------------------------------------------------+
*/

#ifndef __THUTHU2907__
#define __THUTHU2907__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>

#ifdef _WIN32
	#include <winsock2.h>
	#include <WS2tcpip.h>
	#define read(a, b, c) recv(a, b, c, 0)
	#define write(a, b, c) send(a, b, c, 0)
#else
	#include <netinet/in.h>
	#include <sys/socket.h>
#endif

#define SERVER_PORT 2907
#define BUFFER_SIZE 32768
#define FILE_SIZE_T long

void initWS(void)
{
	#ifdef _WIN32
		WSADATA wsa;
		if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
			printf("Failed to initialize WinSock. Error Code: %d", WSAGetLastError());
			exit(EXIT_FAILURE);
		}
	#else
	#endif
}

void cleanWS(int sockfd)
{
	#ifdef _WIN32
		closesocket(sockfd);
		WSACleanup();
	#else
		close(sockfd);
	#endif
}

char* cvt2arr(long n, int nmemb)
{
	char* res = (char*)malloc(nmemb * sizeof(char));
	for (int i = 0; i < nmemb; ++i) {
		*(res + i) = (0xFF & (n >> (8 * i)));
	}
	return res;
}

long cvt2num(char* n, int nmemb)
{
	long res = 0;
	for (int i = 0; i < nmemb; ++i) {
		res |= ((unsigned char)n[i] << (8 * i));
	}
	return res;
}

struct sockaddr_in get_sock_info(uint32_t sockfd)
{
	struct sockaddr_in res;
	socklen_t addr_len = sizeof(res);
	getpeername(sockfd, (struct sockaddr*) &res, &addr_len);
	return res;
}

uint32_t get_sock_ip(uint32_t sockfd)
{
	struct sockaddr_in sockinfo = get_sock_info(sockfd);
	return (uint32_t)(sockinfo.sin_addr.s_addr);
}

uint16_t get_sock_port(uint32_t sockfd)
{
	struct sockaddr_in sockinfo = get_sock_info(sockfd);
	return (uint16_t)(sockinfo.sin_port);
}

bool comp_sock(uint32_t sockfd, uint32_t ip, uint16_t port)
{
	uint32_t sock_ip = get_sock_ip(sockfd);
	uint16_t sock_port = get_sock_port(sockfd);
	return (sock_ip == ip && sock_port == port);
}

char* ip_int2str (uint32_t ip_i)
{
	char* res = (char*)malloc(20);
	sprintf(res, "%hu.%hu.%hu.%hu\0", ip_i & 0xFF, (ip_i >> 8) & 0xFF, (ip_i >> 16) & 0xFF, (ip_i >> 24) & 0xFF);
	return res;
}

#endif
