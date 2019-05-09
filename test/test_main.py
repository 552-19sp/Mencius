""" Executes all of the test specifed in /cases. For test format,
    please see README. """

import os
import subprocess
import sys
import time

MAX_TEST_TIME = 10  # Max amount of time a test should wait to check results.
CLIENT_PROCS = []  # List of client subprocesses.


def pass_test(filename):
    """ Handles the "pass" case for the current test.
        filename -- name of current test file """
    print("{} PASSED\n".format(filename))


def fail_test(filename, err_msg):
    """ Handles the "fail" case for the current test.
        filename -- name of current test file
        err_msg -- descriptin of why test failed """
    print("{} FAILED: {}\n".format(filename, err_msg))


def run_server():
    """ Runs a new client as a subprocess. """
    cmd = "../bin/client"
    CLIENT_PROCS.append(subprocess.Popen(cmd))


def clean_op(dirty_op):
    """ Clean op by removing the preceding 'C#: ' and final \n. """
    return (dirty_op.split(None, 1)[1]).rstrip("\n")


def test_case(filename):
    """ Run a test for a test case.
        filename -- name of current test file """
    # Parse test file for setup information.
    with open(filename) as test_file:
        # Parse test file for setup information.
        # This is temporary. Need to read past the SERVER PORTS to get
        # to the num_clients.
        _ = test_file.readline()

        clients_str = test_file.readline()
        num_clients = int(clients_str.split()[1])

        # This will be used to read in all operations from the test file.
        # ops = test_file.readlines()

        # Run every client as subprocess.
        for _ in range(num_clients):
            # This will be used to filter ops based on clients.
            # client_ops = [op for op in ops if "C{}:".format(client_num) in op]
            run_server()

        # Allow clients to finish workloads.
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

        pass_test(filename)
        return True

def run_tests():
    """ Run all tests specified in the cases directory. """
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

if __name__ == "__main__":
    run_tests()
