# Token Ring network

Token ring is a network topology tailored for low latency and real time networking.

It consists of tying together consecutive nodes forming a ring.

!(TokenRing)[http://www.webopedia.com/FIG/RING.gif]

## Access control

* A token tours through the network in a clockwise fashion, only when its visit
takes place a node is granted permission to pass along a message, otherwise
it shall be retained until the next iteration.

## Features

* In a token ring, message passing is more reliable in terms of throughput and
latency as there are no drawbacks regarding ethernet's contention-based CSMA/CD.

* Nodes are automatically identified and (experimentaly) organise themselves to
form the ring topology, setting the network infrastructure without manual
intervention or parameter-dependent.

* Arbitrarily large messages can be sent. They are split into small frames
and sent one-by-one every once the remitter node seizes the token.

* Noteworthy **TCP/IP** layers are not used. Messages are passed through
bare wire, thus achieving really high speed rates of upwards to _2_ orders
of magnitude higher than that of standard TCP/IP.

## Compilation

First off, you need to set up the number of machines in the network. You may
find this parameter at Makefile at line 12 (-DMAQUINAS\_NA\_REDE).

Lastly, upon binary generation with Makefile compiling tool

```
$ make
```

you should pass along two arguments to the binary in each node

* *host*: network name where token ring nodes should listen.

* *num\_maquina*: node's UID, should be a natural number.

Once they are all set, you'll be able to pipe messages into the ring.

Moreover, make sure ports _2514_ up to _2514+n_, where _n_ is the number of
nodes, are open.

## Protocol

Message frame is defined as

```
   +--------+------+-----------+-------------------+
   | origin | type | data size |       data        |
   +--------+------+-----------+-------------------+
```

where

* Origin: is message's sender.

* Type: indicates whether the message frame is a text message ("T") or a token ("B").

* Data size: the forthcoming field data's length, should be a number of at most 255 bytes.

* Data: plain text.

