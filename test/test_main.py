# Executes all of the test specifed in /cases. For test format,
# please see README.

import os
import subprocess
import sys
import time

MAX_TEST_TIME = 10  # Max amount of time a test should wait to check results.
CLIENT_PROCS = []  # List of client subprocesses.


def pass_test(filename):
    print("{} PASSED\n".format(filename))


def fail_test(filename, err_msg):
    print("{} FAILED: {}\n".format(filename, err_msg))


def run_server(server_ports, client_num, ops):
    # Spawn a subprocess running with provided params.
    # TODO(justin): Provide client with server_ports, client_name, and ops.
    cmd = "../bin/client"
    CLIENT_PROCS.append(subprocess.Popen(cmd))


def clean_op(s):
    # Clean every op by removing the preceding 'C#: ' and final \n.
    return (s.split(None, 1)[1]).rstrip("\n")


def test_case(filename):
    # Parse test file for setup information.
    with open(filename) as f:
        # Parse test file for setup information.
        server_port_str = f.readline()
        server_port = int(server_port_str.split()[2])

        clients_str = f.readline()
        num_clients = int(clients_str.split()[1])
        
        ops = f.readlines()

        # Run every client as subprocess.
        for client_num in range(num_clients):
            client_ops = [op for op in ops if "C{}:".format(client_num) in op]
            clean_client_ops = list(map(clean_op, client_ops))
            run_server(server_port, client_num, clean_client_ops)

        # Allow clients to finish workloads.
        # TODO(justin): Consider configuring this on a per test basis.
        time.sleep(MAX_TEST_TIME)
        clients_finished = True
        
        # Kill any clients still alive (and fail test)
        for proc in CLIENT_PROCS:
            if proc.poll() is None:
                clients_finished = False
                proc.kill()

        if not clients_finished:
            fail_test(filename, "workloads did not finish")
            return False

        # TODO(justin): Check for serializability here.
        
        pass_test(filename)
        return True


if __name__ == "__main__":
    # Keep track if a test fails.
    all_tests_passed = True

    # Find path of /cases directory.
    cases_directory = os.path.join(os.path.dirname(__file__), 'cases')

    # Iterate over all files in cases, handle only text files.
    for filename in os.listdir(cases_directory):
        if filename.endswith(".txt"):
            if not test_case(os.path.join(cases_directory, filename)):
                all_tests_passed = False

    # If tests did not pass, exit with an error.
    if not all_tests_passed:
        sys.exit(1)
