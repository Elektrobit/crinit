FROM ubuntu:jammy

ARG USER=ci
ARG UID=1000
ARG GID=1000

ENV DEBIAN_FRONTEND=noninteractive
RUN dpkg --add-architecture i386
RUN apt-get update \
 && apt-get install -y sudo locales vim screen libncurses-dev \
    build-essential git python3-minimal cmake \
    gcc-aarch64-linux-gnu \
    rpm \
    doxygen graphviz plantuml \
    clang llvm clang-tidy \
    qemu-user-static \
    libc6:i386 \
    libcmocka-dev \
 && rm -rf /var/lib/apt/lists/*

RUN locale-gen en_US.UTF-8 \
 && groupadd -g $GID -o ci \
 && useradd -m -u $UID -g $GID -o -s /bin/bash $USER \
 && echo "$USER ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers

USER $USER
ENV LC_ALL=en_US.UTF-8
ENV DOCKERBUILD=1

CMD /bin/bash

