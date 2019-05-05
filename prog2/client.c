#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "helper.h"

int main (int argc, char **argv) {
    struct addrinfo hints, *server_info, *my_info;
    int socketFD;
    const char *inputFileName = argv[4];
    FILE *inputFile;

    if (argc != 5) {
        printf("Correct format: ./client <server address> <server port number> <client listen port> <file name>\n");
        return 1;
    }

    memset (&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;
    //ip address info
    getaddrinfo(argv[1], argv[2], &hints, &server_info);
    getaddrinfo(NULL, argv[3], &hints, &my_info);

    if (server_info != NULL) {

        socketFD = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);
	printf("Socket created\n");

        struct timeval tv;
	// 1 second timeout
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        setsockopt(socketFD, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv,sizeof(struct timeval));

        bind(socketFD, my_info->ai_addr, my_info->ai_addrlen);

        inputFile = fopen(inputFileName, "rb");

        struct stat inputFileInfo;

	//file size calculations
        stat(inputFileName, &inputFileInfo);

        unsigned int total_frag = ((inputFileInfo.st_size) / MAX_DATA_SIZE) + 1;
        unsigned int current_frag_no = 1;

        char *byteArrayPacket;
        Packet currentPacket;
        currentPacket.total_frag = total_frag;
        strncpy (currentPacket.filename, inputFileName, strlen(inputFileName) + 1);

        while (current_frag_no <= total_frag) {
            currentPacket.frag_no = current_frag_no;

            bool endOfFile = feof(inputFile);
            char nextChar;

            unsigned int i;
            for (i = 0; i < MAX_DATA_SIZE && !endOfFile; i++) {
                nextChar = getc (inputFile);
                if (feof(inputFile)) {
                    endOfFile = true;
                    i--;
                } else {
                    currentPacket.filedata[i] = nextChar;
                }
            }

            currentPacket.filedata[i] = '\0';
            currentPacket.size = i;

            unsigned int currentPacketTotalSize = createPacket (&byteArrayPacket, &currentPacket);

            char buf[BUFLEN];
            int ackReceived = 0;
            Packet ackPacket;

            do {
		//Sent packet
                int bytesSent = sendto(socketFD, byteArrayPacket, currentPacketTotalSize, 0, server_info->ai_addr, sizeof(struct sockaddr_storage));
		printf("Packet sent\n");

                struct sockaddr_storage dummyVar;
                int dummyVar_len = sizeof(dummyVar);

                // Wait 1 second for ACK 
                int bytesReceived = recvfrom(socketFD, buf, BUFLEN, 0, (struct sockaddr *) &dummyVar, &dummyVar_len);
		printf("ACK packet received\n");

                if (bytesReceived != -1) {
                    ackReceived = 1;
                    memset(&ackPacket,0, sizeof(Packet));
                    extractPacket(&ackPacket, buf, bytesReceived);
                    current_frag_no = ackPacket.frag_no;
                }

            } while(ackReceived == 0 && current_frag_no <= total_frag);

            free(byteArrayPacket);
        }

        freeaddrinfo(server_info);
        freeaddrinfo(my_info);
    }

    return 0;
}

