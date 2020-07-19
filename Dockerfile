FROM debian:latest as qjs-build
LABEL maintainer "lane@joeandlane.com"

ARG VERSION=2020-07-05
ARG UID=1000
ARG GID=1000

ENV QJSNET_DEBUG=true

RUN apt update -y && apt install --no-install-recommends -y build-essential gcc wget zip patch bash patch \
    libc6-dev-i386 clang libcurl4-openssl-dev git cmake gcc-multilib ca-certificates libssl-dev nano file net-tools \
    curl telnet

# Reduce bandwidth and build time by placing these in the root of your project
ADD quickjs-${VERSION}.tar.xz .
ADD quickjs-extras-${VERSION}.tar.xz .
# OTHERWISE uncomment these next four lines
#RUN if [ ! -f quickjs-${VERSION}.tar.xz ]; then wget https://bellard.org/quickjs/quickjs-${VERSION}.tar.xz; fi
#RUN if [ ! -f quickjs-extras-${VERSION}.tar.xz ]; then wget https://bellard.org/quickjs/quickjs-extras-${VERSION}.tar.xz; fi
#RUN tar -xf quickjs-extras-${VERSION}.tar.xz
#RUN tar -xf quickjs-${VERSION}.tar.xz

RUN cd /quickjs-${VERSION} && ./release.sh all && make install && cd / && cd /usr/lib && ln -s /usr/local/lib/quickjs . && cd / 

COPY qjs_net /qjs_net/
RUN cd qjs_net && make && cp libqjsnet.so /usr/local/lib/libqjsnet.so && cp qjsnet.h /usr/local/include/

RUN groupadd -g $GID qjs && useradd -ms /bin/bash -g qjs -u $UID -d /home/qjs qjs

# See also https://medium.com/@calbertts/writing-native-modules-in-c-for-quickjs-engine-49043587f2e2

USER qjs
WORKDIR /app
# docker build . -t qjs
# docker run -it -v `pwd`:/app -p 7981:7891 --name qjs --rm qjs /bin/bash
# qjs src/test_server.ts s # Creates the server
# Troubleshoot:
# LD_DEBUG=files qjs src/server.ts
# run:
# ./qjs src/index
