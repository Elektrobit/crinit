ARG REPO
ARG UBUNTU_RELEASE
FROM ${REPO}${REPO:+/}ubuntu:${UBUNTU_RELEASE}

ARG USER=ci
ARG UID=1000
ARG GID=1000

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update \
 && apt-get install -y sudo locales vim screen libncurses-dev \
    build-essential git python3-minimal cmake re2c \
    valgrind \
    rpm wget \
    doxygen graphviz plantuml \
    clang llvm clang-tidy \
    libcmocka-dev \
    bc \
 && rm -rf /var/lib/apt/lists/*

# install debbuild
RUN wget 'https://github.com/debbuild/debbuild/releases/download/22.02.1/debbuild_22.02.1-0ubuntu20.04_all.deb' \
         'https://github.com/debbuild/debbuild-macros/releases/download/0.0.6/debbuild-macros_0.0.6-0ubuntu20.04_all.deb' \
 && apt-get update \
 && apt-get install -y ./debbuild_*.deb ./debbuild-macros_*.deb \
 && rm -rf /var/lib/apt/lists/* \
 && rm -f debbuild_*.deb debbuild-macros_*.deb

RUN locale-gen en_US.UTF-8 \
 && groupadd -g $GID -o ci \
 && useradd -m -u $UID -g $GID -o -s /bin/bash $USER \
 && echo "$USER ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers

USER $USER
ENV LC_ALL=en_US.UTF-8
ENV DOCKERBUILD=1

CMD /bin/bash

