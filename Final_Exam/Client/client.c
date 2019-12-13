/**
 +-------------------------------------------------+
 | Project     : Final Exam                        |
 | Author      : Tung Pham Thanh                   |
 | Student ID  : 17020042                          |
 | Class       : QH - 2017 - I / CQ - C - A - C    |
 +-------------------------------------------------+
*/

#include "../helper.h"

char file_name[256];
FILE* file;
pthread_mutex_t lock;

float receive_file(int32_t servfd) // return how many secs to download this file
{
	int n_bytes;
	char buf[BUFFER_SIZE];
	FILE_SIZE_T file_size = 0;

	int n_peer = 0;
	int32_t *peer = NULL;

	printf("Waiting for file from server...");
	memset(buf, 0, sizeof(buf));

	// receive file name from server
	pthread_mutex_lock(&lock);
	if ((n_bytes = read(servfd, buf, sizeof(buf))) < 0) {
		perror("read file name error");
		exit(EXIT_FAILURE);
	}
	pthread_mutex_unlock(&lock);

	pthread_mutex_lock(&lock);
	strcpy(file_name, "SharedFolder/");
	strcat(file_name, buf);
	file = fopen(file_name, "wb");
	if (file == NULL) {
		printf("Error while opening file to write!: %s\n", file_name);
		exit(EXIT_FAILURE);
	}
	pthread_mutex_unlock(&lock);

	// start clock & downloading file
	printf("\nReceiving file %s...", buf);
	clock_t start_time = clock();

	int32_t src_fd = servfd;
	while (true) {
		pthread_mutex_lock(&lock);
		memcpy(buf, cvt2arr(file_size, sizeof(FILE_SIZE_T)), sizeof(FILE_SIZE_T)); // number of bytes downloaded
		if ((n_bytes = write(src_fd, buf, sizeof(FILE_SIZE_T))) < 0) {
			perror("write offset error");
			exit(EXIT_FAILURE);
		}
		if ((n_bytes = read(src_fd, buf, sizeof(FILE_SIZE_T))) < 0) { // read size of packet
			perror("read packet size error");
			exit(EXIT_FAILURE);
		}
		FILE_SIZE_T pck_size = cvt2num(buf, sizeof(FILE_SIZE_T));
		if (pck_size == 0) { // pck_size == 0 -> receive (ip, port) of client who has downloaded
			if ((n_bytes = read(src_fd, buf, sizeof(uint32_t))) < 0) { // read ip
				perror("read ip error");
				exit(EXIT_FAILURE);
			}
			uint32_t ip = cvt2num(buf, sizeof(uint32_t)); // int type of ip address
			/*
				1. ip == 0 --> end of file
				2. ip != 0 --> continue receiving file segment from other peer
			*/
			if (!ip) {
				pthread_mutex_unlock(&lock);
				break;
			}
			if ((n_bytes = read(src_fd, buf, sizeof(uint16_t))) < 0) { // read port
				perror("read port error");
				exit(EXIT_FAILURE);
			}
			uint16_t port = cvt2num(buf, sizeof(uint16_t));
			printf("receive from %hu\n", port);
			bool connected = false;
			for (int i = 0; i < n_peer; ++i)
				if (comp_sock(peer[i], ip, port)) {
					connected = true;
					src_fd = peer[i];
				}
			if (!connected) {
				struct sockaddr_in peeraddr;
				peeraddr.sin_family = AF_INET;
				peeraddr.sin_port = htons(port);
				peeraddr.sin_addr.s_addr = htonl(ip);

				peer = (int32_t*)realloc(peer, ++n_peer * sizeof(int32_t));
				if ((peer[n_peer - 1] = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
					perror("socket error");
					exit(EXIT_FAILURE);
				}
				if (connect(peer[n_peer - 1], (struct sockaddr*) &peeraddr, sizeof(peeraddr)) < 0) {
					perror("connect to peer error");
					exit(EXIT_FAILURE);
				}

				src_fd = peer[n_peer - 1];
			}
		} else { // pck_size <> 0 -> receive next [pck_size] bytes of file
			if (file == NULL)
				file = fopen(file_name, "ab");
			while (pck_size > 0) {
				if ((n_bytes = read(src_fd, buf, sizeof(buf))) < 0) {
					perror("read error");
					exit(EXIT_FAILURE);
				}
				n_bytes = fwrite(buf, 1, n_bytes, file);
				pck_size -= n_bytes;
				file_size += n_bytes;
			}
			src_fd = servfd;
		}
		pthread_mutex_unlock(&lock);
	}

	// close file & return time elapsed
	clock_t end_time = clock();
	printf("\nSize of downloaded file: %ld bytes\n", file_size);
	fclose(file);
	file = NULL;
	return 1.0 * (end_time - start_time) / CLOCKS_PER_SEC;
}

static void* send_file(void* arg)
{
	int32_t connfd = *((int32_t*) arg);
	pthread_detach(pthread_self());

	int n_bytes;
	char buf[BUFFER_SIZE];

	while (true) {
		if ((n_bytes = read(connfd, buf, sizeof(FILE_SIZE_T))) < 0) { // read offset from peer
			perror("read offset error");
			exit(EXIT_FAILURE);
		}

		pthread_mutex_lock(&lock);
		FILE_SIZE_T offset = cvt2num(buf, sizeof(FILE_SIZE_T));
		fclose(file);
		file = fopen(file_name, "rb");
		fseek(file, 0, SEEK_END);
		FILE_SIZE_T pck_size = ftell(file) - offset;

		char* pck_size_str = (char*)&pck_size;
		if ((n_bytes = write(connfd, pck_size_str, sizeof(FILE_SIZE_T))) < 0) { // send packet size to peer
			perror("send pck_size error");
			exit(EXIT_FAILURE);
		}

		fseek(file, offset, SEEK_SET);
		while (pck_size > 0) { // send segment [offset, EOF] to peer
			n_bytes = fread(buf, 1, sizeof(buf), file);
			if ((n_bytes = write(connfd, buf, n_bytes)) < 0) {
				perror("write error");
				exit(EXIT_FAILURE);
			}
			pck_size -= n_bytes;
		}
		fclose(file);
		file = NULL;
		pthread_mutex_unlock(&lock);
	}
}

static void* listen4peer(void* arg)
{
	int32_t sockfd = *((int32_t*) arg);
	pthread_detach(pthread_self());
	pthread_t tid;
	int32_t* iptr;
	struct sockaddr_in cliaddr;
	socklen_t len = sizeof(cliaddr);

	while (true) {
		iptr = (int32_t*)malloc(sizeof(int32_t));
		*iptr = accept(sockfd, (struct sockaddr*) &cliaddr, &len);
		pthread_create(&tid, NULL, &send_file, (void*)iptr);
	}
}

int main(int argc, char** argv)
{
	initWS();

	if (pthread_mutex_init(&lock, NULL) != 0) {
        printf("Mutex not initialized\n");
        exit(EXIT_FAILURE);
    }

	uint16_t PORT;
	printf("Client port: ");
	scanf("%hu", &PORT);

	char svIP[20];
	printf("Server IP: ");
	scanf("\n%s", svIP);

	int32_t servfd;
	struct sockaddr_in servaddr;

	// create socket file descriptor
	if ((servfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket error");
		exit(EXIT_FAILURE);
	}

	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(SERVER_PORT);
	servaddr.sin_addr.s_addr = inet_addr(svIP);

	if (connect(servfd, (struct sockaddr*) &servaddr, sizeof(servaddr)) < 0) { // connect to server
		perror("connect error");
		exit(EXIT_FAILURE);
	}
	char* listen_port = (char*)&PORT;
	if (write(servfd, listen_port, sizeof(uint16_t)) < 0) { // send listen port to server
		perror("write listen port error");
		exit(EXIT_FAILURE);
	}

	// create another thread to listen from other clients
	int32_t sockfd;
	struct sockaddr_in addr;
	uint32_t addr_len = sizeof(addr);
	char opt = 1;

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("create listen socket error");
		exit(EXIT_FAILURE);
	}

	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
		perror("setsockopt error");
		exit(EXIT_FAILURE);
	}

	addr.sin_family = AF_INET;
	addr.sin_port = htons(PORT);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(sockfd, (struct sockaddr*) &addr, sizeof(addr)) < 0) {
		perror("bind error");
		exit(EXIT_FAILURE);
	}

	if (listen(sockfd, 5) < 0) {
		perror("listen error");
		exit(EXIT_FAILURE);
	}

	pthread_t tid;
	pthread_create(&tid, NULL, &listen4peer, &sockfd);

	// loop receive file
	float total_time = 0.0;

	while (true) {
		float this_time = receive_file(servfd);
		total_time += this_time;
		printf("Last file: %.3f s | Total: %.3f s\n", this_time, total_time);
	}

	pthread_mutex_destroy(&lock);

	return 0;
}