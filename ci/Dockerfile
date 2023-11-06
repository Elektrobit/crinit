ARG REPO
ARG UBUNTU_RELEASE
FROM ${REPO}${REPO:+/}ubuntu:${UBUNTU_RELEASE}

ARG USER=ci
ARG UID=1000
ARG GID=1000

ARG EB_GIT_SOURCES=https://github.com/Elektrobit/

ARG SAFU_REPO_NAME=safu.git
ARG SAFU_REPO_PATH=${EB_GIT_SOURCES}/${SAFU_REPO_NAME}

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update \
 && apt-get install -y sudo locales vim screen git bc \
    libncurses-dev libcmocka-dev libiberty-dev libjson-c-dev libssl-dev libmnl-dev \
    build-essential python3-minimal rpm wget doxygen graphviz plantuml \
    re2c cmake clang llvm clang-tidy pkg-config valgrind \
 && rm -rf /var/lib/apt/lists/*

# Install safu
RUN --mount=type=ssh \
 git clone ${SAFU_REPO_PATH} \
 && cmake -B ${SAFU_REPO_NAME%.*}/build ./${SAFU_REPO_NAME%.*} \
 -DUNIT_TESTS=Off \
 -DCMAKE_BUILD_TYPE=Release \
 -DCMAKE_INSTALL_PREFIX=/usr \
 && make -C ${SAFU_REPO_NAME%.*}/build install

# Install debbuild
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
