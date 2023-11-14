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
    build-essential python-is-python3 python3-venv python3-pip rpm wget doxygen \
    graphviz plantuml re2c cmake clang llvm clang-tidy pkg-config valgrind \
 && rm -rf /var/lib/apt/lists/*

# Install safu
RUN git clone ${SAFU_REPO_PATH} \
 && cmake -B ${SAFU_REPO_NAME%.*}/build ./${SAFU_REPO_NAME%.*} \
 -DUNIT_TESTS=Off \
 -DCMAKE_BUILD_TYPE=Release \
 -DCMAKE_INSTALL_PREFIX=/usr \
 && make -C ${SAFU_REPO_NAME%.*}/build install

# set up sphinx
RUN python -m venv /home/$USER/.venv \
  && . /home/$USER/.venv/bin/activate \
  && python -m pip install \
     beautifulsoup4  \
     myst-parser \
     sphinx \
     sphinx-c-autodoc \
     sphinx-copybutton \
     sphinx-favicon \
     sphinxcontrib-programoutput

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
ENV SPHINX_VENV="/home/$USER/.venv/"

# install sphinx-c-autodoc dependencies
RUN wget -qO- https://apt.llvm.org/llvm-snapshot.gpg.key | sudo tee /etc/apt/trusted.gpg.d/apt.llvm.org.asc \
  && echo "\n\
# for llvm 16 packages\n\
deb http://apt.llvm.org/jammy/ llvm-toolchain-jammy-16 main\n\
deb-src http://apt.llvm.org/jammy/ llvm-toolchain-jammy-16 main\n\
" | sudo tee /etc/apt/sources.list \
  && sudo apt-get update \
  && sudo apt-get install -y libclang1-16 \
  && sudo ln -s $(find / -name libclang-16.so.1 -print -quit) /usr/lib/libclang-16.so

CMD /bin/bash
