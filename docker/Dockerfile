FROM ubuntu:eoan AS base

ARG MAKE_ARGS

RUN apt-get update && apt-get upgrade -y && \
    apt-get install -y git build-essential

WORKDIR /src
RUN git clone git://gcc.gnu.org/git/gcc.git -b master --depth 1 --single-branch

RUN apt-get install -y libmpfr-dev libgmp3-dev libmpc-dev flex

WORKDIR /build/gcc
RUN /src/gcc/configure --disable-multilib --enable-languages=c,c++
RUN make ${MAKE_ARGS}

RUN make install

# WORKDIR /src
# RUN git clone https://github.com/Kitware/CMake.git -b v3.16.5
#
# WORKDIR /build/CMake
# RUN /src/CMake/configure
# RUN make ${MAKE_FLAGS}
# RUN make install


FROM base AS xdev-env

RUN rm -r /src/gcc
RUN rm -r /build/gcc

WORKDIR /root

RUN apt-get update && apt-get upgrade --no-install-recommends -y && \
    apt-get install -y vim cmake sudo python3-pip
RUN pip3 install conan

ENV CC=/usr/local/bin/gcc
ENV CXX=/usr/local/bin/g++

RUN apt-get clean
