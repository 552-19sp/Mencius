CC = g++
CFLAGS = -Wall -g -std=c++14
PROGS = all

all: client server

client: Client.o
	$(CC) $(CFLAGS) -o $@ $^

Client.o: Client.cpp
	$(CC) $(CFLAGS) -c $<

server: Server.o
	$(CC) $(CFLAGS) -o $@ $^

Server.o: Server.cpp
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(PROGS) *.o client server