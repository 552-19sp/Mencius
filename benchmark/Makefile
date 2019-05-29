CC = g++
CFLAGS = -std=c++14 -isystem benchmark/include
LIBS = -Lbuild/src -lpthread -lbenchmark

all:
	$(CC) $(CFLAGS) $(LIBS) Benchmarks.cpp -o benchmarks

install:
	git clone https://github.com/google/benchmark.git benchmark
	git clone https://github.com/google/googletest.git benchmark/googletest
	mkdir -p build
	cd build; cmake ../benchmark; make

clean:
	rm -f benchmarks
