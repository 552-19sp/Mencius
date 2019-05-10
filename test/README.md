# Testing Basics
To run integration tests, from this directory, simply run `python3 test_main.py`. This will execute all of the integration tests specified in the "cases" directory. It will exit with status 1 if any of the tests fail.

# Adding New Tests
Simply create a `*.txt` file in the "cases" directory. The script will pick it up automatically. For naming, please follow the convention: `<num_of_clients>_<test_suite_name>` (i.e. `oneclient_basic.txt`). The testing file format is as follows:

SERVER PORTS \<list of server ports, seperated by a space\>
CLIENTS \<number of clients\>

\# Individual Test Comment
C\<client number\>: PUT \<key\> \<value\>
C\<client number\>: GET \<key\>
C\<client number\>: APPEND \<key\> \<value\>
