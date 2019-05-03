#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <sys/sendfile.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>

#define PORT_NUM 5000
#define SERVER_ADDR "192.168.1.7"
#define FILE_TO_SEND "file.txt"

int main(int argc, char ** argv){
	int serv_sock;
	int client_sock;
	socklen_t sock_len;
	ssize_t len;
	struct sockaddr_in serv_addr;
	struct sockaddr_in client_addr;
	int fd;
	int sent_bytes = 0;
	char file_size[256];
	struct stat file_stat;
	int offset;
	int remain_data;

	serv_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (server_socket == -1){
		fprintf(stderr, "Error making socket %s", strerror(errno));
		exit(EXIT_FAILURE);
	}

	memset(&server_addr, 0 sizeof(server_addr));
	server_addr.sin_samily = AF_INET;
	inet_pton(AF_INET, SERVER_ADDR, &(server_addr.sin_addr));
	server_addr.sin_port = htons(PORT_NUMBER);

	if((bind(server_socket, (struct sockaddr *)&server_addr, sizeof(struct sockaddr))) == -1){
		fprintf(stderr, "Error binding %s", strerror(errno));
		exit(EXIT_FAILURE);
	}
	if((listen(server_socket, 5)) == -1){	
		fprintf(stderr, "Error listening %s", strerror(errno));
		exit(EXIT_FAILURE);
	}
	fd = open(FILE_TO_SEND, O_RDONLY);
	if(fd == -1){
		fprintf(stderr, "Error opening file --> %s", strerror(errno));
		exit(EXIT_FAILURE);
	}
	fprintf(stdout, "File size: %d bytes\n", file_stat.st_size);
	sock_len = sizeof(struct sockaddr_in);
	client_sock = accept(server_socket, (struct sockaddr *)&client_addr, &sock_len);	
	if(client_sock == -1){
		fprintf(stderr,"Error on accept %s",strerror(errno));
		exit(EXIT_FAILURE);
	}
	printf("Accepted a client %s\n", inet_ntoa(peer_addr.sin_addr));
	len = send(client_sock, file_size, sizeof(file_size),0);
	if(len < 0){
		fprintf(stderr,"Error on sending %s",strerror(errno));
		exit(EXIT_FAILURE);
	}
	printf("Server sent %d bytes\n", len);
	offset = 0;
	remain_data = file_stat.st_size;
	while(((sent_bytes = sendfile(peer_socket, fd, &offset, BUFSIZ)) > 0) && (remain_data > 0)){
		printf("1. Server sent %d bytes, offset is now %d, remaining data is %d\n",sent_bytes, offset, remain_data);
		remain_data -= sent_bytes;
		printf("2. Server sent %d bytes, offset is now %d, remaining data is %d\n",sent_bytes, offset, remain_data);
	}
	close(client_sock);
	close(serv_sock);
	return 0;
}