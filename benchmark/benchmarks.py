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

NO_FAILURE_CODE = 0
RANDOM_FAILURE_CODE = 1

NO_DROP_RATE = 0

CLIENT_EXE = "../bin/client"


def run_background_clients(num_clients, num_servers, drop_rate):
    """ Runs extra clients, does not wait for them to exit. """
    for _ in range(num_clients):
        joined_ops = ','.join([CMD] * NUM_CLIENT_OPS)
        CLIENT_PROCS.append(subprocess.Popen(
            (CLIENT_EXE, str(num_servers), str(drop_rate), joined_ops), stdout=subprocess.PIPE))


def run_benchmark_client(num_servers, drop_rate, failure_code):
    """ Runs a new benchmark client as a subprocess. """
    joined_ops = ','.join([CMD] * NUM_BENCH_OPS)
    subprocess.call(
        [CLIENT_EXE, str(num_servers), str(drop_rate), str(failure_code), joined_ops],
        stdout=subprocess.PIPE)


def run_benchmark(num_clients, num_servers, drop_rate, random_failures, writer):
    """ Runs a no failure benchmark with the specified parameters. """
    # Setup background clients, if any.
    num_background_clients = num_clients - 1
    run_background_clients(num_background_clients, num_servers, drop_rate)

    # Encode if random server failures should be allowed.
    failure_code = RANDOM_FAILURE_CODE if random_failures else NO_FAILURE_CODE

    # Time benchmarking client.
    start = time.time()
    run_benchmark_client(num_servers, drop_rate, failure_code)
    end = time.time()

    # Kill background clients, if any.
    for proc in CLIENT_PROCS:
        if proc.poll() is None:
            proc.kill()

    avg_latency = round((end - start) * 1000 / NUM_BENCH_OPS)
    print("random failures: {}/{} clients/{} servers/{} drop rate: {} ms".format(
        failure_code, num_clients, num_servers, drop_rate, avg_latency))
    writer.writerow([failure_code, num_clients, num_servers, drop_rate, avg_latency])


def run_tcp_mencius_benchmarks(writer):
    """ Runs all the benchmarks for TCP Mencius. """
    writer.writerow(['Mencius', 'TCP'])
    for num_clients in NUM_CLIENTS:
        for num_servers in NUM_SERVERS:
            run_benchmark(num_clients, num_servers, NO_DROP_RATE, False, writer)
    for num_clients in NUM_CLIENTS:
        for num_servers in NUM_SERVERS:
            run_benchmark(num_clients, num_servers, NO_DROP_RATE, True, writer)

def run_benchmark_suite(writer):
    """ Run all benchmarks. """
    print("Starting TCP benchmarks...")
    run_tcp_mencius_benchmarks(writer)


if __name__ == "__main__":
    with open('results.csv', mode='w') as output_file:
        run_benchmark_suite(
            csv.writer(output_file, delimiter=',', quotechar='"', quoting=csv.QUOTE_MINIMAL))
