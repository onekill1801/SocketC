/**
 +-------------------------------------------------+
 | Project     : Final Exam                        |
 | Author      : Tung Pham Thanh                   |
 | Student ID  : 17020042                          |
 | Class       : QH - 2017 - I / CQ - C - A - C    |
 +-------------------------------------------------+
*/

#include "../helper.h"

int n_client = 0; // number of connected client
int32_t* clifd = NULL; // list of connected client
uint16_t* cli_port = NULL; // list of listen port of client
FILE_SIZE_T* offset_l = NULL; // list offset of connected client
FILE* file;
char file_name[256];
pthread_mutex_t lock;
int n_active_peer = 0; // number of client DOWNLOADING file
clock_t start_time;
float total_time = 0.0;

static void* proc(void* arg)
{
	int32_t connfd = *((int32_t*) arg);
	pthread_detach(pthread_self());

	int n_bytes;
	char buf[BUFFER_SIZE];

	while (true) {
		if (start_time == 0)
			continue;

		// reset offset
		pthread_mutex_lock(&lock);
		for (int i = 0; i < n_client; ++i)
			if (clifd[i] == connfd) {
				offset_l[i] = 0;
				break;
			}
		++n_active_peer;

		// send file name to client
		if ((n_bytes = write(connfd, file_name, strlen(file_name))) < 0) {
			perror("write file name error");
			exit(EXIT_FAILURE);
		}
		pthread_mutex_unlock(&lock);

		// open file to send to client
		pthread_mutex_lock(&lock);
		if (file == NULL) {
			strcpy(buf, "SharedFolder/");
			strcat(buf, file_name);
			file = fopen(buf, "rb");
			if (file == NULL) {
				printf("Error while opening file!\n");
				exit(EXIT_FAILURE);
			}
		}
		pthread_mutex_unlock(&lock);

		while (true) {
			pthread_mutex_lock(&lock);

			FILE_SIZE_T cur_off = ftell(file);
			if ((n_bytes = read(connfd, buf, sizeof(FILE_SIZE_T))) < 0) {
				perror("read offset error");
				exit(EXIT_FAILURE);
			}
			FILE_SIZE_T cli_off = cvt2num(buf, sizeof(FILE_SIZE_T));

			char pck_size[sizeof(FILE_SIZE_T)];
			memset(pck_size, 0, sizeof(pck_size));

			if (cli_off == cur_off) { // request new segment
				if (feof(file)) { // end of file --> send 0
					if ((n_bytes = write(connfd, pck_size, sizeof(FILE_SIZE_T))) < 0) { // write pck_size
						perror("write pck_size error");
						exit(EXIT_FAILURE);
					}
					if ((n_bytes = write(connfd, pck_size, sizeof(FILE_SIZE_T))) < 0) { // eof signal (0)
						perror("write eof error");
						exit(EXIT_FAILURE);
					}
					pthread_mutex_unlock(&lock);
					break;
				} else {
					FILE_SIZE_T pck_size_i = fread(buf, 1, sizeof(buf), file);
					memcpy(pck_size, cvt2arr(pck_size_i, sizeof(FILE_SIZE_T)), sizeof(FILE_SIZE_T));
					if ((n_bytes = write(connfd, pck_size, sizeof(FILE_SIZE_T))) < 0) {
						perror("write pck_size error");
						exit(EXIT_FAILURE);
					}

					// send file segment
					if ((n_bytes = write(connfd, buf, pck_size_i)) < 0) {
						perror("write error");
						exit(EXIT_FAILURE);
					}

					// update offset
					for (int i = 0; i < n_client; ++i)
						if (clifd[i] == connfd) {
							offset_l[i] += n_bytes;
							break;
						}
				}
			} else { // redirect to client who downloaded this segment
				if ((n_bytes = write(connfd, pck_size, sizeof(FILE_SIZE_T))) < 0) { // write pck_size
					perror("write pck_size error");
					exit(EXIT_FAILURE);
				}
				for (int i = 0; i < n_client; ++i) {
					if (offset_l[i] < cli_off)
						continue;
					uint32_t ip = htonl(get_sock_ip(clifd[i]));
					uint16_t port = cli_port[i];
					memcpy(buf, cvt2arr(ip, sizeof(uint32_t)), sizeof(uint32_t));
					memcpy(buf + sizeof(uint32_t), cvt2arr(port, sizeof(uint16_t)), sizeof(uint16_t));
					if ((n_bytes = write(connfd, buf, sizeof(uint32_t) + sizeof(uint16_t))) < 0) {
						perror("write peer info error");
						exit(EXIT_FAILURE);
					}
				}
			}

			pthread_mutex_unlock(&lock);
		}

		pthread_mutex_lock(&lock);
		--n_active_peer;
		if (n_active_peer == 0) {
			clock_t end_time = clock();
			float elapsed_time = 1.0 * (end_time - start_time) / CLOCKS_PER_SEC;
			total_time += elapsed_time;
			printf("Last time: %.3f s | Total: %.3f s\n", elapsed_time, total_time);
			start_time = 0;
			fclose(file);
			file = NULL;
		}
		pthread_mutex_unlock(&lock);
		while (n_active_peer > 0);
	}
}

static void* listen4cli(void* arg)
{
	int32_t sockfd = *((int32_t*) arg);
	pthread_detach(pthread_self());

	pthread_t tid;
	struct sockaddr_in cliaddr;
	int32_t* iptr;

	socklen_t len = sizeof(cliaddr);

	while (true) {
		iptr = (int32_t*)malloc(sizeof(int32_t));
		*iptr = accept(sockfd, (struct sockaddr*) &cliaddr, &len);

		// add client's file descriptor and offset
		clifd = (int32_t*)realloc(clifd, ++n_client * sizeof(int32_t));
		cli_port = (uint16_t*)realloc(cli_port, n_client * sizeof(int32_t));
		clifd[n_client - 1] = *iptr;
		char* listen_port = (char*)(cli_port + n_client - 1);
		if (read(*iptr, listen_port, sizeof(uint16_t)) < 0) { // get listen port of client
			perror("read listen port error");
			exit(EXIT_FAILURE);
		}
		cli_port[n_client - 1] = cvt2num(listen_port, sizeof(uint16_t));
		offset_l = (FILE_SIZE_T*)realloc(offset_l, n_client * sizeof(FILE_SIZE_T));

		// create new thread for this client
		pthread_create(&tid, NULL, &proc, (void*)iptr);
	}
}

int main(int argc, char** argv)
{
	initWS();

	if (pthread_mutex_init(&lock, NULL) != 0) {
        printf("Mutex not initialized\n");
        exit(EXIT_FAILURE);
    }

	int32_t sockfd;
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket error");
		exit(EXIT_FAILURE);
	}

	struct sockaddr_in sockaddr;
	memset(&sockaddr, 0, sizeof(sockaddr));
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_port = htons(SERVER_PORT);
	sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);

	char opt = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
		perror("setsockopt error");
		exit(EXIT_FAILURE);
	}

	if (bind(sockfd, (struct sockaddr*) &sockaddr, sizeof(sockaddr)) < 0) {
		perror("bind error");
		exit(EXIT_FAILURE);
	}

	if (listen(sockfd, 5) < 0) {
		perror("listen error");
		exit(EXIT_FAILURE);
	}

	pthread_t listen_tid;
	pthread_create(&listen_tid, NULL, &listen4cli, (void*)&sockfd);

	// select file will be sent to client
	while (true) {
		pthread_mutex_lock(&lock);
		if (start_time > 0) {
			pthread_mutex_unlock(&lock);
			continue;
		}
		printf("Filename to upload: ");
		scanf("%[^\n]%*c", file_name);
		start_time = clock();
		pthread_mutex_unlock(&lock);
	}
	
	pthread_mutex_destroy(&lock);

	return 0;
}
