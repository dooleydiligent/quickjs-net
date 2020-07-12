## A network library for quickjs
- This is a work in progress.  It doesn't actually function, so keep that in mind if you participate

The goal is to provide basic network (AF_NET) functionality to [quickjs](https://bellard.org/quickjs/quickjs.html) for later exploration of an Express-like module and/or [deno](https://deno.land/) - like functionality

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
Socket1 is 3
Socket2 is 4
Bind to port 3000 successful!
Could not bind to socket on localhost:3000null
```
I'm not yet sure that that **null** is behind 3000