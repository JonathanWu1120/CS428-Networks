#include <stdio.h>  
#include <string.h>   //strlen  
#include <stdlib.h>  
#include <errno.h>  
#include <unistd.h>   //close  
#include <arpa/inet.h>    //close  
#include <sys/types.h>  
#include <sys/socket.h>  
#include <netinet/in.h>  
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros  
#include "port.h"
#define TRUE   1  
#define FALSE  0  

int main(int argc , char *argv[])   
{   
	int opt = TRUE;   
	int master_socket , addrlen , new_socket , client_socket[30] ,  
	    max_clients = 30 , activity, i , valread , sd;   
	int max_sd;   
	struct sockaddr_in address;   
	char buffer[1025];  //data buffer of 1K  
	fd_set readfds;   
	for (i = 0; i < max_clients; i++)   
	{   
		client_socket[i] = 0;   
	}   
	if( (master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0)   
	{   
		perror("socket failed");   
		exit(EXIT_FAILURE);   
	}   
	if( setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt,  
				sizeof(opt)) < 0 )   
	{   
		perror("setsockopt");   
		exit(EXIT_FAILURE);   
	}   
	address.sin_family = AF_INET;   
	address.sin_addr.s_addr = inet_addr("0.0.0.0");   
	address.sin_port = htons( PORT );   
	if (bind(master_socket, (struct sockaddr *)&address, sizeof(address))<0)   
	{   
		perror("bind failed");   
		exit(EXIT_FAILURE);   
	}   
	printf("Listener on port %d \n", PORT);   
	if (listen(master_socket, 3) < 0)   
	{   
		perror("listen");   
		exit(EXIT_FAILURE);   
	}   
	addrlen = sizeof(address);   
	puts("Waiting for connections ...");   
	int n; 
	while(TRUE)   
	{   
		FD_ZERO(&readfds);   
		FD_SET(master_socket, &readfds);   
		max_sd = master_socket;   
		for ( i = 0 ; i < max_clients ; i++)   
		{   
			sd = client_socket[i];   
			if(sd > 0)   
				FD_SET( sd , &readfds);   
			if(sd > max_sd)   
				max_sd = sd;   
		}   
		activity = select( max_sd + 1 , &readfds , NULL , NULL , NULL);   
		if ((activity < 0) && (errno!=EINTR))   
		{   
			printf("select error");   
		}   
		if (FD_ISSET(master_socket, &readfds))   
		{   
			if ((new_socket = accept(master_socket,  
							(struct sockaddr *)&address, (socklen_t*)&addrlen))<0)   
			{   
				perror("accept");   
				exit(EXIT_FAILURE);   
			}   
			printf("New connection , socket fd is %d , ip is : %s , port : %d\n" , new_socket , inet_ntoa(address.sin_addr) , ntohs (address.sin_port));   
			for (i = 0; i < max_clients; i++)   
			{   
				if( client_socket[i] == 0 )   
				{   
					client_socket[i] = new_socket;   
					printf("Adding to list of sockets as %d\n" , i);   
					break;   
				}   
			}   
		}   
		for (i = 0; i < max_clients; i++)   
		{   
			sd = client_socket[i];   

			if (FD_ISSET( sd , &readfds))   
			{   
				valread = read( sd , buffer, 1024);   
				if (valread == 0)   
				{   
					//Somebody disconnected , get his details and print  
					getpeername(sd , (struct sockaddr*)&address ,(socklen_t*)&addrlen);   
					printf("Host disconnected , ip %s , port %d \n" ,inet_ntoa(address.sin_addr) , ntohs(address.sin_port));   

					close( sd );   
					client_socket[i] = 0;   
				}   
				else 
				{   
					printf("From client in spot %d:%s\nTo client %d:", i,buffer, i);
					bzero(buffer,1024);
					n = 0;
					while((buffer[n++] = getchar()) != '\n')
						;
					write(sd,buffer,1024);
				}

			}   
		}
		if(strncmp("exit",buffer,4) == 0){
			printf("Exit\n");
			break;

		}   
	}   
	close(sd);
	return 0;   
}   




