CC = gcc
CFLAGS = -Wall -g

all: serveur client

serveur: serveur.o
	$(CC) $(CFLAGS) -o serveur serveur.o -lpthread

client: client.o
	$(CC) $(CFLAGS) -o client client.o -lpthread

serveur.o: serveur.c
	$(CC) $(CFLAGS) -c serveur.c 

client.o: client.c
	$(CC) $(CFLAGS) -c client.c

clean:
	rm -f *.o serveur client

.PHONY: all clean