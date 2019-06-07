TCP_CLIENT_TARGET = client
TCP_SERVER_TARGET = server

UDP_CLIENT_TARGET = client_udp
UDP_SERVER_TARGET = server_udp

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

TCP_SERVER_SRC = $(wildcard $(SRCDIR)/tcp/server/*.cpp)
TCP_SERVER_OBJ = $(TCP_SERVER_SRC:$(SRCDIR)/tcp/server/%.cpp=$(OBJDIR)/tcp/server/%.o)

UDP_SERVER_SRC = $(wildcard $(SRCDIR)/udp/server/*.cpp)
UDP_SERVER_OBJ = $(UDP_SERVER_SRC:$(SRCDIR)/udp/server/%.cpp=$(OBJDIR)/udp/server/%.o)

UTIL_SRC = $(wildcard $(SRCDIR)/util/*.cpp) $(wildcard $(SRCDIR)/util/*/*.cpp)
UTIL_OBJ = $(UTIL_SRC:$(SRCDIR)/util/%.cpp=$(OBJDIR)/%.o)

MESSAGE_SRC = $(wildcard $(SRCDIR)/message/*.cpp)
MESSAGE_OBJ = $(MESSAGE_SRC:$(SRCDIR)/message/%.cpp=$(OBJDIR)/%.o)

all: $(TCP_CLIENT_TARGET) $(TCP_SERVER_TARGET) $(UDP_CLIENT_TARGET) $(UDP_SERVER_TARGET)

tcp: $(TCP_CLIENT_TARGET) $(TCP_SERVER_TARGET)

udp: $(UDP_CLIENT_TARGET) $(UDP_SERVER_TARGET)

# Link TCP executables
$(TCP_CLIENT_TARGET): obj/tcp/client/Client.o $(UTIL_OBJ) $(MESSAGE_OBJ)
	$(CC) $^ -o $(BINDIR)/$@ $(LIBS)

$(TCP_SERVER_TARGET): $(TCP_SERVER_OBJ) $(UTIL_OBJ) $(MESSAGE_OBJ)
	$(CC) $^ -o $(BINDIR)/$@ $(LIBS)

# Link UDP executables
$(UDP_CLIENT_TARGET): obj/udp/client/Client.o $(UTIL_OBJ) $(MESSAGE_OBJ)
	$(CC) $^ -o $(BINDIR)/$@ $(LIBS)

$(UDP_SERVER_TARGET): $(UDP_SERVER_OBJ) $(UTIL_OBJ) $(MESSAGE_OBJ)
	$(CC) $^ -o $(BINDIR)/$@ $(LIBS)

# Compile general object files
$(OBJDIR)/%.o: $(SRCDIR)/util/%.cpp
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/%.o: $(SRCDIR)/message/%.cpp
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/KVStore/%.o: $(SRCDIR)/util/KVStore/%.cpp
	$(CC) $(CFLAGS) -c $< -o $@

# TCP src -> obj
$(OBJDIR)/tcp/client/Client.o: $(SRCDIR)/tcp/client/Client.cpp | $(OBJDIR) $(BINDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/tcp/server/%.o: $(SRCDIR)/tcp/server/%.cpp | $(OBJDIR) $(BINDIR)
	$(CC) $(CFLAGS) -c $< -o $@

# UDP src -> obj
$(OBJDIR)/udp/client/Client.o: $(SRCDIR)/udp/client/Client.cpp | $(OBJDIR) $(BINDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/udp/server/%.o: $(SRCDIR)/udp/server/%.cpp | $(OBJDIR) $(BINDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR) $(BINDIR):
	mkdir -p $@ obj/tcp obj/tcp/client obj/tcp/server \
	    obj/udp obj/udp/client obj/udp/server obj/KVStore

cpplint:
	cpplint --filter=-runtime/references,-build/c++11 $(shell find . -name \*.h?? -or -name \*.cpp | grep -vE "^\.\/benchmark\/")

ctags:
	ctags -R --exclude=benchmark .

.PHONY: clean
clean:
	rm -rf $(OBJDIR) $(BINDIR)
