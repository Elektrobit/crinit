ARG REPO
ARG UBUNTU_RELEASE
FROM ${REPO}${REPO:+/}ubuntu:${UBUNTU_RELEASE}

RUN groupdel -f ubuntu && userdel -r ubuntu || true

ARG USER=ci
ARG UID=1000
ARG GID=1000

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update \
    && apt-get install -y \
    bash-completion \
    bc \
    build-essential \
    clang \
    clang-format \
    clang-tidy \
    cmake \
    debhelper \
    devscripts \
    doxygen \
    equivs \
    fakeroot \
    flake8 \
    git \
    git-buildpackage \
    graphviz \
    keyutils \
    libcap-dev \
    libcmocka-dev \
    libiberty-dev \
    libjson-c-dev \
    libmbedtls-dev \
    libmnl-dev \
    libncurses-dev \
    libssl-dev \
    llvm \
    locales \
    pkg-config \
    plantuml \
    python-is-python3 \
    python3-pip \
    python3-venv \
    re2c \
    rpm \
    screen \
    shellcheck \
    shfmt \
    software-properties-common \
    sudo \
    valgrind \
    vim \
    wget \
    npm \
    yapf3 \
    && rm -rf /var/lib/apt/lists/* \
    && ln -fs /usr/share/zoneinfo/UTC /etc/localtime

# Install all build dependencies on elos
RUN add-apt-repository -y ppa:elos-team/ppa \
    && apt-get install -y libelos-dev \
    && rm -rf /var/lib/apt/lists/*

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
    && groupadd -g $GID ci \
    && useradd -m -u $UID -g $GID -s /bin/bash $USER \
    && echo "$USER ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers

# Install commitlint too with conventional commit config and dependencies.
RUN npm install n -g \
    && n stable
RUN npm install @commitlint/cli -g \
    && npm install @commitlint/config-conventional -g

# Install doctoc
RUN npm install doctoc -g

USER $USER
ENV LC_ALL=en_US.UTF-8
ENV DOCKERBUILD=1
ENV SPHINX_VENV="/home/$USER/.venv/"

# install sphinx-c-autodoc dependencies
RUN wget -qO- https://apt.llvm.org/llvm-snapshot.gpg.key | sudo tee -a /etc/apt/trusted.gpg.d/apt.llvm.org.asc \
    && echo "\n\
    # for llvm 16 packages\n\
    deb http://apt.llvm.org/jammy/ llvm-toolchain-jammy-16 main\n\
    deb-src http://apt.llvm.org/jammy/ llvm-toolchain-jammy-16 main\n\
    " | sudo tee -a /etc/apt/sources.list \
    && sudo apt-get update \
    && sudo apt-get install -y libclang1-16 \
    && sudo ln -s $(find / -name libclang-16.so.1 -print -quit) /usr/lib/libclang-16.so

CMD /bin/bash
