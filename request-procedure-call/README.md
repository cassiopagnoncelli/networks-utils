A remote procedure call (RPC) is a networks tool so that a client, by means of
a request, asks a server to perform operations, in this case arithmetic, returning
the result through _in tandem_ peers.

Machines are connected in a string wherein one end is the client and the other is
the server which will perform the operation.

## Execute it

As for the server node, run

```
./servidor port
```

whereas for other nodes run

```
./servidor hostname:host_port node_port
```

## Compilation

Binary generation is automated through Makefile, type

```
$ make
```

and that should suffice provided you have a C compiler in your machine.
