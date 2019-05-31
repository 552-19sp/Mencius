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

UTIL_SRC = $(wildcard $(SRCDIR)/util/*.cpp) $(wildcard $(SRCDIR)/util/*/*.cpp)
UTIL_OBJ = $(UTIL_SRC:$(SRCDIR)/util/%.cpp=$(OBJDIR)/%.o)

MESSAGE_SRC = $(wildcard $(SRCDIR)/message/*.cpp)
MESSAGE_OBJ = $(MESSAGE_SRC:$(SRCDIR)/message/%.cpp=$(OBJDIR)/%.o)

all: client server

client: obj/client/Client.o $(UTIL_OBJ) $(MESSAGE_OBJ)
	$(CC) $^ -o $(BINDIR)/$@ $(LIBS)

server: $(SERVER_OBJ) $(UTIL_OBJ) $(MESSAGE_OBJ)
	$(CC) $^ -o $(BINDIR)/$@ $(LIBS)

$(OBJDIR)/%.o: $(SRCDIR)/util/%.cpp
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/%.o: $(SRCDIR)/message/%.cpp
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/KVStore/%.o: $(SRCDIR)/util/KVStore/%.cpp
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/client/Client.o: $(SRCDIR)/client/Client.cpp | $(OBJDIR) $(BINDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/server/%.o: $(SRCDIR)/server/%.cpp | $(OBJDIR) $(BINDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR) $(BINDIR):
	mkdir -p $@ obj/client obj/server obj/KVStore

cpplint:
	cpplint --filter=-runtime/references,-build/c++11 $(shell find . -name \*.h?? -or -name \*.cpp | grep -vE "^\.\/benchmark\/")

.PHONY: clean
clean:
	rm -rf $(OBJDIR) $(BINDIR)
