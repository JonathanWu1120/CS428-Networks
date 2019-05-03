#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <errno.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>

#define PORT_NUMBER 5000
#define SERVER_ADDR "192.168.1.7"
#define FILENAME "/home/hello.c"

int main(int argc, char **argv){
	int client_socket;
	ssize_t len;
	struct sockaddr_in remote_addr;
	char buffer[BUFSIZ];
	int file_size;
	FILE *received_file;
	int remain_data = 0;
	
	memset(&remote_addr, 0, sizeof(remote_addr));
	remote_addr.sin_family = AF_INET;
	inet_pton(AF_INET, SERVER_ADDR, &(remote_addr.sin_addr));
	remote_addr.sin_port = htons(PORT_NUMBER);

	client_socket = socket(AF_INET, SOCK_STREAM,0);
	if(client_socket == -1){
		fprintf(stderr, "ERROR CREATING SOCKET %s", strerror(errno));
		exit(EXIT_FAILURE);
	}
	if(connect(client_socket, (struct sockaddr *)&remote_addr, sizeof(struct sockaddr))==-1){
		fprintf(stderr,"Failed to connect %s", strerror(errno));
		exit(EXIT_FAILURE);
	}
	recv(client_socket, buffer, BUFSIZ, 0);
	file_size = atoi(buffer);

	received_file = fopen(FILENAME, "W");
	if(received_file == NULL){
		fprintf(stderr, "Failed opening file %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	remain_data = file_size;
	while((remain_data > 0) && ((len = recv(client_socket, buffer, BUFSIZ, 0)) > 0)){
		fwrite(buffer, sizeof(char), len, received_file);
		remain_data -= len;
		printf("Received %ld bytes and %d remaining\n",len, remain_data);
	}
	fclose(received_file);
	close(client_socket);
	return 0;
}

