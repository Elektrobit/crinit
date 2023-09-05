ARG REPO
ARG UBUNTU_RELEASE
FROM ${REPO}${REPO:+/}ubuntu:${UBUNTU_RELEASE}

ARG USER=ci
ARG UID=1000
ARG GID=1000

#ARG EMLIX_GIT_SOURCES=git@github.com:emlix/
ARG EMLIX_GIT_SOURCES=git@gitlabintern.emlix.com:elektrobit/base-os

ARG CMOCKA_EXTENSIONS_REPO_NAME=cmocka-extensions.git
ARG CMOCKA_EXTENSIONS_REPO_PATH=${EMLIX_GIT_SOURCES}/${CMOCKA_EXTENSIONS_REPO_NAME}
ARG CMOCKA_EXTENSIONS_REPO_REF=integration

ARG CMOCKA_MOCKS_REPO_NAME=cmocka-mocks.git
ARG CMOCKA_MOCKS_REPO_PATH=${EMLIX_GIT_SOURCES}/${CMOCKA_MOCKS_REPO_NAME}
ARG CMOCKA_MOCKS_REPO_REF=integration

ARG SAFU_REPO_NAME=safu.git
ARG SAFU_REPO_PATH=${EMLIX_GIT_SOURCES}/${SAFU_REPO_NAME}

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update \
 && apt-get install -y sudo locales vim screen git bc \
    libncurses-dev libcmocka-dev libiberty-dev libjson-c-dev libssl-dev libmnl-dev \
    build-essential python3-minimal rpm wget doxygen graphviz plantuml \
    re2c cmake clang llvm clang-tidy pkg-config valgrind \
 && rm -rf /var/lib/apt/lists/*

RUN mkdir -p -m 0700 /root/.ssh \
    && ssh-keyscan github.com >> /root/.ssh/known_hosts \
    && ssh-keyscan gitlabintern.emlix.com >> /root/.ssh/known_hosts

# Install cmocka-extensions
RUN --mount=type=ssh \
 git clone ${CMOCKA_EXTENSIONS_REPO_PATH} \
 && cmake -B ${CMOCKA_EXTENSIONS_REPO_NAME%.*}/build ./${CMOCKA_EXTENSIONS_REPO_NAME%.*} \
 -DUNIT_TESTS=OFF \
 -DCMAKE_BUILD_TYPE=Release \
 -DCMAKE_INSTALL_PREFIX=/usr \
 && make -C ${CMOCKA_EXTENSIONS_REPO_NAME%.*}/build install

# Install cmocka-mocks
RUN --mount=type=ssh \
 git clone ${CMOCKA_MOCKS_REPO_PATH} \
 && cmake -B ${CMOCKA_MOCKS_REPO_NAME%.*}/build ./${CMOCKA_MOCKS_REPO_NAME%.*} \
 -DUNIT_TESTS=OFF \
 -DCMAKE_BUILD_TYPE=Release \
 -DCMAKE_INSTALL_PREFIX=/usr \
 -DCMOCKA_EXTENSIONS_URI=${CMOCKA_EXTENSIONS_REPO_PATH} \
 -DCMOCKA_EXTENSIONS_REF=${CMOCKA_EXTENSIONS_REPO_REF} \
 && make -C ${CMOCKA_MOCKS_REPO_NAME%.*}/build install

# Install safu
RUN --mount=type=ssh \
 git clone ${SAFU_REPO_PATH} \
 && cmake -B ${SAFU_REPO_NAME%.*}/build ./${SAFU_REPO_NAME%.*} \
 -DUNIT_TEST=OFF \
 -DCMAKE_BUILD_TYPE=Release \
 -DCMAKE_INSTALL_PREFIX=/usr \
 -DCMOCKA_EXTENSIONS_URI=${CMOCKA_EXTENSIONS_REPO_PATH} \
 -DCMOCKA_EXTENSIONS_REF=${CMOCKA_EXTENSIONS_REPO_REF} \
 -DCMOCKA_MOCKS_URI=${CMOCKA_MOCKS_REPO_PATH} \
 -DCMOCKA_MOCKS_REF=${CMOCKA_MOCKS_REPO_REF} \
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
