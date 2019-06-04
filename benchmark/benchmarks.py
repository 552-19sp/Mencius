""" Executes a series of latency benchmarks against TCPCLient. """

import subprocess
import time
import csv

NUM_CLIENTS = [1, 2, 3] # Number of clients to benchmark with.
NUM_SERVERS = [3, 5] # Number of servers each test cluster should have.
NUM_BENCH_OPS = 10 # Number of ops to test per benchmark.
NUM_CLIENT_OPS = 1000 # Number of background client ops.

CMD = "GET A"
CLIENT_PROCS = []
MAX_TIME = 10

NO_FAILURE_CODE = 0
GRADUAL_FAILURE_CODE = 1
INSTANT_FAILURE_CODE = 2

CLIENT_EXE = "../bin/client"


def run_background_clients(num_clients, num_servers, drop_rate):
    """ Runs extra clients, does not wait for them to exit. """
    for _ in range(num_clients):
        joined_ops = ','.join([CMD] * NUM_CLIENT_OPS)
        CLIENT_PROCS.append(subprocess.Popen(
            (CLIENT_EXE, str(num_servers), str(drop_rate), joined_ops), stdout=subprocess.PIPE))


def run_benchmark_client(num_servers, drop_rate):
    """ Runs a new benchmark client as a subprocess. """
    joined_ops = ','.join([CMD] * NUM_BENCH_OPS)
    subprocess.call(
        [CLIENT_EXE, str(num_servers), str(drop_rate), joined_ops], stdout=subprocess.PIPE)


def run_no_failure_benchmark(num_clients, num_servers, drop_rate, writer):
    """ Runs a no failure benchmark with the specified parameters. """
    # Setup background clients, if any.
    num_background_clients = num_clients - 1
    run_background_clients(num_background_clients, num_servers, drop_rate)

    # Time benchmarking client.
    start = time.time()
    run_benchmark_client(num_servers, drop_rate)

    # Kill background clients, if any.
    for proc in CLIENT_PROCS:
        if proc.poll() is None:
            proc.kill()

    end = time.time()
    avg_latency = round((end - start) * 1000 / NUM_BENCH_OPS)
    print("no failures/{} clients/{} servers/{} drop rate: {} ms".format(
        num_clients, num_servers, drop_rate, avg_latency))
    writer.writerow([NO_FAILURE_CODE, num_clients, num_servers, drop_rate, avg_latency])

def run_tcp_mencius_benchmarks(writer):
    """ Runs all the benchmarks for TCP Mencius. """
    writer.writerow(['Mencius', 'TCP'])
    for num_clients in NUM_CLIENTS:
        for num_servers in NUM_SERVERS:
            run_no_failure_benchmark(num_clients, num_servers, 0, writer)

def run_benchmark_suite(writer):
    """ Run all benchmarks. """
    print("Starting TCP benchmarks...")
    run_tcp_mencius_benchmarks(writer)


if __name__ == "__main__":
    with open('results.csv', mode='w') as output_file:
        run_benchmark_suite(
            csv.writer(output_file, delimiter=',', quotechar='"', quoting=csv.QUOTE_MINIMAL))
