# Network utils

A collection of networks services, protocols, and architectures.

For each project, browse it for a more detailed presentation.

## #!(Token Ring network)[./token-ring]

Token ring is a network topology tailored to achieve low latency and
real time networking at the cost of some reliability.

Written in pure ANSI C.

## Hi-ADSD network architecture

Hi-ADSD is a hypercube network architecture aimed to dramatically reduce
the number of messages passed in the network to O(log N) exchanges.

It features a few other pure distributed systems techniques to achieve
fail resilience and self remedy.

Written in pure ANSI C at UFPR Computer Science labs.

## Remote Procedure Call

A server-client architecture wherein a client passes along a formula to
a string of machines until reaching the remote server which will evaluate
and return its result.

Network can handle as many layer nodes as wanted between client and server.

Written in pure ANSI C.

## FTP server-client

Written in C++ using *raw sockets*, messages are passed through bare wire
in direct connections by the use of a crossover cable.

It works hierarchicaly below IP protocol mimicking TCP/IP functionalities
like parity-checking, sliding windows, package splitting, error correction,
interruption resilience, among others.

