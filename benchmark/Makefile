CC = g++
CFLAGS = -std=c++11 -isystem benchmark/include
LIBS = -Lbuild/src -lpthread -lbenchmark

all: 
	$(CC) $(CFLAGS) $(LIBS) Benchmarks.cpp -o benchmarks
clean:
	rm -f benchmarks