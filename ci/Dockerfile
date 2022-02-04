FROM ubuntu:focal

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
 && rm -rf /var/lib/apt/lists/*

# build and install cmocka as a static lib for aarch64+musl
COPY aarch64-cmocka-toolchain.cmake /opt/src/
RUN mkdir -p /opt/src/cmocka \
 && git clone https://git.cryptomilk.org/projects/cmocka.git /opt/src/cmocka \
 && cd /opt/src/cmocka \
 && git checkout 5a4b15870efa2225e6586fbb4c3af05ff0659434 \
 && mkdir -p build \
 && cd build \
 && cmake \
        -DCMAKE_TOOLCHAIN_FILE=/opt/src/aarch64-cmocka-toolchain.cmake \
        -DPICKY_DEVELOPER=ON \
        -DBUILD_SHARED_LIBS=ON \
        -DWITH_EXAMPLES=OFF \
        -DCMAKE_INSTALL_PREFIX=/usr/aarch64-linux-gnu \
        .. \
 && make \
 && make install \
 && rm -rf /opt/src/cmocka

RUN locale-gen en_US.UTF-8 \
 && groupadd -g $GID -o ci \
 && useradd -m -u $UID -g $GID -o -s /bin/bash $USER \
 && echo "$USER ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers

USER $USER
ENV LC_ALL=en_US.UTF-8
ENV DOCKERBUILD=1

CMD /bin/bash

