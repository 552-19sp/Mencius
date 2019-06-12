# Mencius

Implementation of [Mencius](https://www.usenix.org/legacy/events/osdi08/tech/full_papers/mao/mao.pdf), a Paxos-like state machine replication protocol, for [CSE 552 19sp](https://courses.cs.washington.edu/courses/cse552/19sp/). Both UDP and TCP variants are included.

## Run
- Run `make` to build the project.
- The only dependency is Boost 1.68.

Four executables will be created in `bin`: `client` (TCP client), `server` (TCP server), `client_udp` (UDP client), `server_udp` (UDP server).

### TCP

To run the TCP server, run the executable `server` and pass a port to listen on.

```
./bin/server 11111
```

To run the TCP client, run the executable `client` with the following arguments:

```
./bin/client <number-of-servers> <server-drop-rate> <random-failure-bit> <operations>
```

where `<operations>` is a series of comma separated operations. For example, you could pass `PUT foo bar,GET foo`.

### UDP

Follow the instructions for TCP but replace the client executable with `client_udp` and the server executable with `server_udp`.

A helper script `scripts/launch.py` will simultaneously launch multiple server instances.
