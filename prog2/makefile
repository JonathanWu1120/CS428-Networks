CC=gcc

all: client server

client: client.o helper.o
	$(CC) -g  client.o helper.o -o client 

server: server.o helper.o
	$(CC) -g ./server/server.o helper.o -o ./server/server

helper.o: helper.c helper.h
	$(CC) -g helper.c -c

client.o: client.c
	$(CC) -g client.c -c

server.o: server.c
	$(CC) -g ./server/server.c -o ./server/server.o

clean:
	rm -f client server *o
