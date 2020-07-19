## A network library for quickjs
- This is a work in progress.  It doesn't actually function, so keep that in mind if you participate

The goal is to provide basic network (AF_NET) functionality to [quickjs](https://bellard.org/quickjs/quickjs.html) for later exploration of an Express-like module and/or [deno](https://deno.land/) - like functionality
## TODO
- Use non-blocking socket communication to allow us to continue single-threaded behavior
- Get a better understanding of js_std_loop so the application can remain running even when there are no pending jobs

## Build
```
docker build . -t qjs
```
We assume that your linux user:group is 1000:1000.  

Override this assumption with

--build-arg UID=1000 --build-arg GID=1000

on the build command.  Or you could use the -u 1000:1000 option on the run command, below.
## Run
```
docker run -it -v `pwd`:/app -p 7981:7891 --name qjs qjs /bin/bash
```

By default we publish port 7981 as an homage to [khanhas](https://github.com/khanhas/minnet-quickjs) who got me thinking about this in the first place

This docker build is NOT optimized.  It produces an image of about .8 jiggle bytes ATM.

// TODO: make a two stage build to remove cmake et al and copy the image into a "scratch" base container. Maybe just install Fabrice's [binaries](https://bellard.org/quickjs/binary_releases) and be done with it.
## Test
Admitedly there is no testing going on here.

I'm hopeful that the future will include testing for the C module(s) as well as the javascript modules.

As it turns out there is no test harness for quickjs.  So that field is wide open!

But anyway:
```
docker run -it -v `pwd`:/app -p 7981:7981 --name qjs --rm qjs /bin/bash
# then
qjs src/test_socket.js
# Output:
AF_INET is 2
AF_INET6 is 10
Beginning
Validate the server can be instantiated with no parameters (e.g. new Server())
QJS_NET - [DEBUG] - qjsnet_server_ctor()
server instantiated
QJS_NET - [DEBUG] - qjsnet_server_get_prop(PORT=>7981)
QJS_NET - [DEBUG] - qjsnet_server_get_prop(PORT=>7981)
QJS_NET - [DEBUG] - qjsnet_server_get_prop(PORT=>7981)
Found port 7981
QJS_NET - [DEBUG] - qjsnet_server_get_prop(HOST=127.0.0.1)
QJS_NET - [DEBUG] - qjsnet_server_get_prop(HOST=127.0.0.1)
QJS_NET - [DEBUG] - qjsnet_server_close()
QJS_NET - [DEBUG] - All memory released
Validate the server can be instantiated with only a port (e.g. new Server(3000))
QJS_NET - [DEBUG] - Port set to 3000
QJS_NET - [DEBUG] - qjsnet_server_ctor(3000)
Checking port
QJS_NET - [DEBUG] - qjsnet_server_get_prop(PORT=>3000)
Port is 3000
QJS_NET - [DEBUG] - qjsnet_server_get_prop(PORT=>3000)
QJS_NET - [DEBUG] - qjsnet_server_get_prop(PORT=>3000)
Checking address
QJS_NET - [DEBUG] - qjsnet_server_get_prop(HOST=127.0.0.1)
QJS_NET - [DEBUG] - qjsnet_server_get_prop(HOST=127.0.0.1)
closing
QJS_NET - [DEBUG] - qjsnet_server_close()
QJS_NET - [DEBUG] - All memory released
closed
Validate the server can be instantiated with a port and an host (e.g. new Server(4000, "localhost"))
QJS_NET - [DEBUG] - Port set to 4000
QJS_NET - [DEBUG] - qjsnet_get_address(localHost)
QJS_NET - [DEBUG] - qjsnet_server_ctor(4000, 127.0.0.1)
QJS_NET - [DEBUG] - qjsnet_server_get_prop(PORT=>4000)
QJS_NET - [DEBUG] - qjsnet_server_get_prop(PORT=>4000)
QJS_NET - [DEBUG] - qjsnet_server_get_prop(HOST=127.0.0.1)
QJS_NET - [DEBUG] - qjsnet_server_get_prop(HOST=127.0.0.1)
QJS_NET - [DEBUG] - qjsnet_server_close()
QJS_NET - [DEBUG] - All memory released
Validate the server can be instantiated with a port and an address (e.g. new Server(5000, '0.0.0.0'))
QJS_NET - [DEBUG] - Port set to 5000
QJS_NET - [DEBUG] - qjsnet_get_address(0.0.0.0)
QJS_NET - [DEBUG] - qjsnet_server_ctor(5000, 0.0.0.0)
QJS_NET - [DEBUG] - qjsnet_server_get_prop(PORT=>5000)
QJS_NET - [DEBUG] - qjsnet_server_get_prop(PORT=>5000)
QJS_NET - [DEBUG] - qjsnet_server_get_prop(HOST=0.0.0.0)
Address is 0.0.0.0
QJS_NET - [DEBUG] - qjsnet_server_get_prop(HOST=0.0.0.0)
QJS_NET - [DEBUG] - qjsnet_server_get_prop(HOST=0.0.0.0)
QJS_NET - [DEBUG] - qjsnet_server_get_prop(HOST=0.0.0.0)
QJS_NET - [DEBUG] - qjsnet_server_get_prop(HOST=0.0.0.0)
QJS_NET - [DEBUG] - qjsnet_server_close()
QJS_NET - [DEBUG] - All memory released
Validate the server can be instantiated with a port and an address (e.g. new Server(5000, '192.168.1.2'))
QJS_NET - [DEBUG] - Port set to 5000
QJS_NET - [DEBUG] - qjsnet_get_address(0.0.0.0)
QJS_NET - [DEBUG] - qjsnet_server_ctor(5000, 0.0.0.0)
QJS_NET - [DEBUG] - qjsnet_server_get_prop(PORT=>5000)
QJS_NET - [DEBUG] - qjsnet_server_get_prop(PORT=>5000)
QJS_NET - [DEBUG] - qjsnet_server_get_prop(HOST=0.0.0.0)
Address is 0.0.0.0
QJS_NET - [DEBUG] - qjsnet_server_get_prop(HOST=0.0.0.0)
QJS_NET - [DEBUG] - qjsnet_server_get_prop(HOST=0.0.0.0)
QJS_NET - [DEBUG] - qjsnet_server_get_prop(HOST=0.0.0.0)
QJS_NET - [DEBUG] - qjsnet_server_get_prop(HOST=0.0.0.0)
QJS_NET - [DEBUG] - qjsnet_server_close()
QJS_NET - [DEBUG] - All memory released
Validate the server cannot be instantiated with a port and an invalid address (e.g. new Server(6000, '999.999.999.999'))
QJS_NET - [DEBUG] - Port set to 6000
QJS_NET - [DEBUG] - qjsnet_get_address(999.999.999.999)
Validate the server can be instantiated and start listening in a single step
QJS_NET - [DEBUG] - qjsnet_server_ctor()
QJS_NET - [DEBUG] - setting callback listening
QJS_NET - [DEBUG] - callback listening is ready
QJS_NET - [DEBUG] - qjsnet_server_listen()
QJS_NET - [DEBUG] - qjsnet_server_listen(7981, 127.0.0.1) - created socket 3
QJS_NET - [DEBUG] - qjsnet_server_listen(7981,127.0.0.1) - listening
QJS_NET - [DEBUG] - qjsnet_raise_event
QJS_NET - [DEBUG] - Invoking the callback: listening
QJS_NET - [DEBUG] - freeing event_name: listening
QJS_NET - [DEBUG] - qjsnet_server_get_prop(HOST=127.0.0.1)
QJS_NET - [DEBUG] - qjsnet_server_get_prop(HOST=127.0.0.1)
QJS_NET - [DEBUG] - qjsnet_server_close()
QJS_NET - [DEBUG] - closing socket 3
QJS_NET - [DEBUG] - Deleting the event from the list
QJS_NET - [DEBUG] - Releasing callback code for event listening
QJS_NET - [DEBUG] - Releasing callback listening
QJS_NET - [DEBUG] - All memory released
If you can see this then it passed
Done
```
