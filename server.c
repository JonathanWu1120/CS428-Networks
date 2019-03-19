#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/errno.h>
#include <sys/wait.h>
#include "port.h"

#ifndef ERESTART
#define ERESTART EINTR
#endif

extern int errno;
void serve(int port);

int main(int argc, char ** argv){
	extern char *optarg;
	extern int optind;
	int c, err = 0; 
	int port = SERVICE_PORT;
	static char usage[] = "usage: %s [-d] [-p port]\n";

	while ((c = getopt(argc, argv, "dp:")) != -1)
		switch (c) {
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
	serve(port);
}


void serve(int port){
	int svc;        /* listening socket providing service */
	int rqst;       /* socket accepting the request */
	socklen_t alen;       /* length of address structure */
	struct sockaddr_in my_addr;    /* address of this service */
	struct sockaddr_in client_addr;  /* client's address */
	int sockoptval = 1;
	char hostname[128]; /* host name, for debugging */
	int num;

	gethostname(hostname, 128);

	/* get a tcp/ip socket */
	/*   AF_INET is the Internet address (protocol) family  */
	/*   with SOCK_STREAM we ask for a sequenced, reliable, two-way */
	/*   conenction based on byte streams.  With IP, this means that */
	/*   TCP will be used */

	if ((svc = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("cannot create socket");
		exit(1);
	}

	/* we use setsockopt to set SO_REUSEADDR. This allows us */
	/* to reuse the port immediately as soon as the service exits. */
	/* Some operating systems will not allow immediate reuse */
	/* on the chance that some packets may still be en route */
	/* to the port. */

	setsockopt(svc, SOL_SOCKET, SO_REUSEADDR, &sockoptval, sizeof(int));

	/* set up our address */
	/* htons converts a short integer into the network representation */
	/* htonl converts a long integer into the network representation */
	/* INADDR_ANY is the special IP address 0.0.0.0 which binds the */
	/* transport endpoint to all IP addresses on the machine. */

	memset((char*)&my_addr, 0, sizeof(my_addr));  /* 0 out the structure */
	my_addr.sin_family = AF_INET;   /* address family */
	my_addr.sin_port = htons(port);
	my_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	/* bind to the address to which the service will be offered */
	if (bind(svc, (struct sockaddr *)&my_addr, sizeof(my_addr)) < 0) {
		perror("bind failed");
		exit(1);
	}

	/* set up the socket for listening with a queue length of 5 */
	if (listen(svc, 5) < 0) {
		perror("listen failed");
		exit(1);
	}

	printf("server started on %s, listening on port %d\n", hostname, port);

	/* loop forever - wait for connection requests and perform the service */
	alen = sizeof(client_addr);     /* length of address */
	char buffer[10241];
	char *buff;
	for (;;) {
		while ((rqst = accept(svc,
		                (struct sockaddr *)&client_addr, &alen)) < 0) {
			/* we may break out of accept if the system call */
			/* was interrupted. In this case, loop back and */
			/* try again */
			if ((errno != ECHILD) && (errno != ERESTART) && (errno != EINTR)) {
				perror("accept failed");
				exit(1);
			}
		}

		printf("received a connection from: %s port %d\n",
			inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
		if ((num = recv(rqst,buffer,1024,0))==-1){
			perror("recv");
			exit(1);
		}else if(num ==0){
			printf("Connection closed\n");
			break;
		}
		buffer[num] = '\0';
		printf("Server: Msg received %s \n", buffer);
		if((send(rqst,buffer,strlen(buffer),0))==-1){
			fprintf(stderr,"Failure sending message\n");
			shutdown(rqst,2);
			break;
		}
		printf("Server:Msg being sent: %s\nNumber of bytes send %d\n",buffer,strlen(buffer));
        	shutdown(rqst, 2);    /* close the connection */
	}
	close(svc);
}
