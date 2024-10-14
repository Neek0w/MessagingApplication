CC = gcc
CFLAGS = -Wall -g

all: server client

server: server.o database.o
	$(CC) $(CFLAGS) -o server server.o database.o -lpthread

client: client.o
	$(CC) $(CFLAGS) -o client client.o -lpthread

server.o: server.c database.h
	$(CC) $(CFLAGS) -c server.c 

database.o: database.c
	$(CC) $(CFLAGS) -c database.c

client.o: client.c
	$(CC) $(CFLAGS) -c client.c

clean:
	rm -f *.o server client

redo: clean all

.PHONY: all clean