CLIENT_TARGET = client
SERVER_TARGET = server

CC = g++
CFLAGS = -Wall -g -std=c++14
LIBS = -pthread -L/usr/include/boost -lboost_system -lboost_chrono
PROGS = all

SRCDIR = src
OBJDIR = obj
BINDIR = bin

# Since we need two executables (but right now each
# is just a single file), use two separate rules.
# If they ever grow  to be multiple files, can move
# to a folder and change this rule to include
# everything in the folder.
CLIENT_SRC = $(wildcard $(SRCDIR)/Client.cpp)
CLIENT_OBJ = $(CLIENT_SRC:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)

SERVER_SRC = $(wildcard $(SRCDIR)/Server.cpp)
SERVER_OBJ = $(SERVER_SRC:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)

all: $(CLIENT_TARGET) $(SERVER_TARGET)

$(CLIENT_TARGET): $(CLIENT_OBJ)
	$(CC) $^ -o $(BINDIR)/$@ $(LIBS)

$(SERVER_TARGET): $(SERVER_OBJ)
	$(CC) $^ -o $(BINDIR)/$@ $(LIBS)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIR) $(BINDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR) $(BINDIR):
	mkdir -p $@

clean:
	rm -rf $(OBJDIR) $(BINDIR)
