#include <sys/socket.h>
#include <stdlib.h>	
#include <stdio.h>	
#include <string.h>
#include <netinet/in.h>	
#include <string.h>
#include <netdb.h>
#include "port.h"
#define MAXSIZE 1024
int conn(char *host,int port);
void disconn(fd);
int debug = 1;

int main(int argc, char**argv){
	extern char *optarg;
	extern int optind;
	int c, err = 0; 
	char *prompt = 0;
	int port = SERVICE_PORT;	
	char *host = "localhost";	
	int fd;				
	char buffer[1024];
	char buff[1024];
	int num;
	static char usage[] = 
	              "usage: %s [-d] [-h serverhost] [-p port]\n";

	while ((c = getopt(argc, argv, "dh:p:")) != -1)
		switch (c) {
		case 'h':  
			host = optarg;
			break;
		case 'p': 
			port = atoi(optarg);
			if (port < 1024 || port > 65535) {
				fprintf(stderr, "invalid port number: %s\n", optarg);
				err = 1;
			}
			break;
		case '?':
			err = 1;
			break;
		}
	if (err || (optind < argc)) {
		fprintf(stderr, usage, argv[0]);
		exit(1);
	}

	printf("connecting to %s, port %d\n", host, port);

	if ((fd = conn(host, port)) < 0)    
		exit(1);   

	/* in a useful program, we would do something here involving reads and writes on fd */
	while(1){
		printf("Client: Enter data for server side:\n");
		fgets(buffer,MAXSIZE-1,stdin);
		if((send(fd,buffer,strlen(buffer),0)) == -1){
			fprintf(stderr, "Message faield to send\n");
			disconn(fd);
			exit(1);
		}else{
			printf("Client: Message being sent: %s\n",buffer);
			num = recv(fd, buffer, sizeof(buffer),0);
			if (num <= 0){
				printf("Error occured\n");
				break;
			}
			buff[num] = '\0';
			printf("Client:Message received from server: %s\n", buffer);
		}
	}

	disconn(fd);   
	return 0;
}


int conn(char *host, int port){
	struct hostent *hp;	/* host information */
	unsigned int alen;	/* address length when we get the port number */
	struct sockaddr_in myaddr;	/* our address */
	struct sockaddr_in servaddr;	/* server address */
	int fd;  /* fd is the file descriptor for the connected socket */

	if (debug) printf("conn(host=\"%s\", port=\"%d\")\n", host, port);

	if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("cannot create socket");
		return -1;
	}

	memset((char *)&myaddr, 0, sizeof(myaddr));
	myaddr.sin_family = AF_INET;
	myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	myaddr.sin_port = htons(0);

	if (bind(fd, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
		perror("bind failed");
		close(fd);
		return -1;
	}

	/* this part is for debugging only - get the port # that the operating */
	/* system allocated for us. */
        alen = sizeof(myaddr);
        if (getsockname(fd, (struct sockaddr *)&myaddr, &alen) < 0) {
                perror("getsockname failed");
		close(fd);
		return -1;
        }
	if (debug) printf("local port number = %d\n", ntohs(myaddr.sin_port));

	/* fill in the server's address and data */
	/* htons() converts a short integer to a network representation */

	memset((char*)&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(port);

	/* look up the address of the server given its name */
	hp = gethostbyname(host);
	if (!hp) {
		fprintf(stderr, "could not obtain address of %s\n", host);
		close(fd);
		return -1;
	}

	/* put the host's address into the server address structure */
	memcpy((void *)&servaddr.sin_addr, hp->h_addr_list[0], hp->h_length);

	/* connect to server */
	if (connect(fd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
		perror("connect failed");
		close(fd);
		return -1;
	}
	if (debug) printf("connected socket = %d\n", fd);
	return fd;
}

/* disconnect from the service */
/* lame: we can just as easily do a shutdown() or close() ourselves */

void disconn(int fd){
	if (debug) printf("disconn(%d)\n", fd);
	shutdown(fd, 2);    /* 2 means future sends & receives are disallowed */
}
