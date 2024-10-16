CC = gcc
CFLAGS = -Wall -g

all: server client server2 client2 drive

drive:
	mkdir -p server/drive/
	mkdir -p server2/drive/

client2: client/client2.o
	$(CC) $(CFLAGS) -o client/client2.exe client/client2.o -lpthread

client/client2.o: client/client2.c
	$(CC) $(CFLAGS) -c client/client2.c -o client/client2.o

server2: server2/server.o server2/database.o
	$(CC) $(CFLAGS) -o server2/server.exe server2/server.o server2/database.o -lpthread

server2/server.o: server2/server.c server2/database.h
	$(CC) $(CFLAGS) -c server2/server.c -o server2/server.o

server2/database.o: server2/database.c	
	$(CC) $(CFLAGS) -c server2/database.c -o server2/database.o

server: server/server.o server/database.o
	$(CC) $(CFLAGS) -o server/server.exe server/server.o server/database.o -lpthread

client: client/client.o
	$(CC) $(CFLAGS) -o client/client.exe client/client.o -lpthread

server/server.o: server/server.c server/database.h
	$(CC) $(CFLAGS) -c server/server.c -o server/server.o

server/database.o: server/database.c
	$(CC) $(CFLAGS) -c server/database.c -o server/database.o

client/client.o: client/client.c
	$(CC) $(CFLAGS) -c client/client.c -o client/client.o

clean:
	rm -f server/*.o client/*.o server/server.exe client/client.exe

redo: clean all

.PHONY: all clean