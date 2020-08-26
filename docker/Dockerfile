FROM ubuntu:focal AS base

ARG MAKE_ARGS

RUN apt-get update && apt-get upgrade -y && \
    apt-get install -y git \
    build-essential \
    cmake \
    g++-10 \
    vim \
    sudo \
    python3-pip \
    tar \
    curl

######### base env #########
FROM base AS xdev-env

RUN pip3 install conan

RUN update-alternative --install /usr/bin/cc cc /usr/bin/gcc-10 100
RUN update-alternative --install /usr/bin/c++ c++ /usr/bin/g++-10 100

RUN apt-get clean

WORKDIR /root

######### qt env #########
FROM xdev-env AS xdev-env-qt

WORKDIR /src
RUN git clone https://github.com/qt/qt5.git --recurse-submodules
WORKDIR /build/qt5
RUN cmake -DCMAKE_BUILD_TYPE=Release \
          /src/qt5 && \
    make ${MAKE_ARGS} && make install

WORKDIR /src
RUN git clone https://github.com/qt-creator/qt-creator.git
WORKDIR /build/qt-creator
RUN cmake -DCMAKE_BUILD_TYPE=Release \
          /src/qt-creator && \
    make ${MAKE_ARGS} && make install


RUN rm -r /src/qt5 /build/qt5 /src/qt-creator /build/qt-creator

RUN apt-get clean && rm -rf /var/lib/apt/lists/*

WORKDIR /root
