CC = gcc
CFLAGS = -Wall -g -Ishared
LDFLAGS = -lpthread

all: directories server client server2 client2

directories:
	mkdir -p region1/server/drive/
	mkdir -p region2/server2/drive/
	mkdir -p obj/

server: region1/server/server.exe

region1/server/server.exe: obj/server.o obj/database.o obj/server_utils.o obj/socket_utils.o
	$(CC) $(CFLAGS) -o region1/server/server.exe obj/server.o obj/database.o obj/server_utils.o obj/socket_utils.o

obj/server.o: region1/server/server.c shared/database.h shared/server_utils.h shared/socket_utils.h
	$(CC) $(CFLAGS) -c region1/server/server.c -o obj/server.o

client: region1/client/client.exe

region1/client/client.exe: obj/client.o obj/client_utils.o obj/socket_utils.o
	$(CC) $(CFLAGS) -o region1/client/client.exe obj/client.o obj/client_utils.o obj/socket_utils.o

obj/client.o: region1/client/client.c shared/client_utils.h shared/socket_utils.h
	$(CC) $(CFLAGS) -c region1/client/client.c -o obj/client.o

server2: region2/server2/server2.exe

region2/server2/server2.exe: obj/server2.o obj/database.o obj/server_utils.o obj/socket_utils.o
	$(CC) $(CFLAGS) -o region2/server2/server2.exe obj/server2.o obj/database.o obj/server_utils.o obj/socket_utils.o

obj/server2.o: region2/server2/server2.c shared/database.h shared/server_utils.h shared/socket_utils.h
	$(CC) $(CFLAGS) -c region2/server2/server2.c -o obj/server2.o

client2: region2/client2/client2.exe

region2/client2/client2.exe: obj/client2.o obj/client_utils.o obj/socket_utils.o
	$(CC) $(CFLAGS) -o region2/client2/client2.exe obj/client2.o obj/client_utils.o obj/socket_utils.o

obj/client2.o: region2/client2/client2.c shared/client_utils.h shared/socket_utils.h
	$(CC) $(CFLAGS) -c region2/client2/client2.c -o obj/client2.o

obj/database.o: shared/database.c shared/database.h
	$(CC) $(CFLAGS) -c shared/database.c -o obj/database.o

obj/server_utils.o: shared/server_utils.c shared/server_utils.h shared/socket_utils.h
	$(CC) $(CFLAGS) -c shared/server_utils.c -o obj/server_utils.o

obj/client_utils.o: shared/client_utils.c shared/client_utils.h shared/socket_utils.h
	$(CC) $(CFLAGS) -c shared/client_utils.c -o obj/client_utils.o

obj/socket_utils.o: shared/socket_utils.c shared/socket_utils.h
	$(CC) $(CFLAGS) -c shared/socket_utils.c -o obj/socket_utils.o

clean: clean_files clean_bin

clean_files:
	rm -rf region1/server/drive/* region2/server2/drive/* region1/client/downloads/* region2/client2/downloads/*

clean_bin:
	rm -f obj/*.o region1/server/server.exe region1/client/client.exe region2/server2/server2.exe region2/client2/client2.exe

redo: clean all