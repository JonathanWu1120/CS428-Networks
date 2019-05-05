
#ifndef HELPER_H
#define HELPER_H

#define MAX_DATA_SIZE 1000		
#define MAX_DATA_SIZE_DIGITS 4	
#define NUM_COLONS 4	
#define MAX_FRAG_DIGITS 10 
#define MAX_FILE_NAME_SIZE 100   
#define BUFLEN 5000		

typedef struct packet {
    unsigned int total_frag;
    unsigned int frag_no;
    unsigned int size;
    char filename[MAX_FILE_NAME_SIZE];
    char filedata[MAX_DATA_SIZE + 1];
} Packet;

unsigned int getNumDigits(int baseTenNum);

void extractIndivStrings (const char * mainStr, char * output, int * counter);

void extractPacket (Packet * result, const char * byteArray, int totalPacketSize);

unsigned int createPacket (char **byteArray, const Packet* packet);

#endif
