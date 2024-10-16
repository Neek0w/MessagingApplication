CC = gcc
CFLAGS = -Wall -g -Ishared
LDFLAGS = -lpthread

all: server client server2 client2 drive

drive:
	mkdir -p region1/server/drive/
	mkdir -p region2/server2/drive/

server: region1/server/server.exe

region1/server/server.exe: obj/server.o obj/database.o
	$(CC) $(CFLAGS) -o region1/server/server.exe obj/server.o obj/database.o

obj/server.o: region1/server/server.c shared/database.h
	$(CC) $(CFLAGS) -c region1/server/server.c -o obj/server.o


client: region1/client/client.exe

region1/client/client.exe: obj/client.o
	$(CC) $(CFLAGS) -o region1/client/client.exe obj/client.o $(LDFLAGS)

obj/client.o: region1/client/client.c
	$(CC) $(CFLAGS) -c region1/client/client.c -o obj/client.o

server2: region2/server2/server2.exe

region2/server2/server2.exe: obj/server2.o obj/database.o
	$(CC) $(CFLAGS) -o region2/server2/server2.exe obj/server2.o obj/database.o $(LDFLAGS)

obj/server2.o: region2/server2/server.c shared/database.h
	$(CC) $(CFLAGS) -c region2/server2/server.c -o obj/server2.o

obj/database.o: shared/database.c shared/database.h
	$(CC) $(CFLAGS) -c shared/database.c -o obj/database.o

client2: region2/client2/client2.exe

region2/client2/client2.exe: obj/client2.o
	$(CC) $(CFLAGS) -o region2/client2/client2.exe obj/client2.o $(LDFLAGS)

obj/client2.o: region2/client2/client2.c
	$(CC) $(CFLAGS) -c region2/client2/client2.c -o obj/client2.o

clean: clean_files clean_bin



clean_files:
	rm -rf region1/server/drive/* region2/server2/drive/* region1/client/downloads/* region2/client2/downloads/*

clean_bin:
	rm -f obj/*.o  region1/server/server.exe region1/client/client.exe region2/server2/server2.exe region2/client2/client2.exe

redo: clean all