CLIENT_TARGET = client
SERVER_TARGET = server

CC = g++
CFLAGS = -Wall -g -std=c++14 -I include/
SHARED_LIBS = -lboost_serialization
LIBS = -pthread -L/usr/include/boost -lboost_system -lboost_chrono $(SHARED_LIBS)
PROGS = all

SRCDIR = src
OBJDIR = obj
BINDIR = bin

# On Mac, Boost is installed through brew, so the
# explicit linking of some libraries is not needed.
ifneq ($(OS),Windows_NT)
    UNAME := $(shell uname -s)
    ifeq ($(UNAME),Darwin)
        LIBS = $(SHARED_LIBS)
    endif
endif

SERVER_SRC = $(wildcard $(SRCDIR)/server/*.cpp)
SERVER_OBJ = $(SERVER_SRC:$(SRCDIR)/server/%.cpp=$(OBJDIR)/server/%.o)

all: client server

client: obj/client/Client.o obj/Message.o obj/Utilities.o
	$(CC) $^ -o $(BINDIR)/$@ $(LIBS)

server: $(SERVER_OBJ) obj/Message.o obj/Utilities.o
	$(CC) $^ -o $(BINDIR)/$@ $(LIBS)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/client/Client.o: $(SRCDIR)/client/Client.cpp | $(OBJDIR) $(BINDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/server/Server.o: $(SERVER_SRC) | $(OBJDIR) $(BINDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR) $(BINDIR):
	mkdir -p $@ obj/client obj/server

cpplint:
	cpplint --filter=-runtime/references --recursive */*

clean:
	rm -rf $(OBJDIR) $(BINDIR)
