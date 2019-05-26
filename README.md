# Mencius

Implementation of [Mencius](https://www.usenix.org/legacy/events/osdi08/tech/full_papers/mao/mao.pdf), a Paxos-like state machine replication protocol, for [CSE 552 19sp](https://courses.cs.washington.edu/courses/cse552/19sp/).

## Run
- Run `make` to build the project.
- The only dependency is Boost 1.68.

Two executables will be created in `bin`: `client` and `server`.

To run `client`, pass a series of space separated commands as the workload.

To run `server`, pass a port to listen on. Alternatively, run `scripts/launch.py` to simultaneously launch multiple server instances.
