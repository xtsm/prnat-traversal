## prnat-traversal

This is a simple example of traversing port-restricted NAT using rendezvous server with private IP address.

Since it's a simple example, a lot of things like acks and keepalive are not implemented.

### Build

Building was only tested on Linux but should probably work on any Unix-like OS.
```
$ mkdir build
$ cd build
$ cmake ..
$ make
```

### Example

Let's say we have a publicly available server at 14.88.22.8. Then we can run this on server:
```
$ ./server 1234
```
Now we can take two hosts and run this command on each of them:
```
$ ./client 14.88.22.8:1234
```
The server automatically couples clients by sending another party's address to each of them.

If we do it quick enough so that NAT doesn't forget about one of the clients, clients will start exchanging "Hello" messages with each other. The behavior is defined in `client.cpp` and can be changed to something more useful like sending actual data.