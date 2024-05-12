FROM debian:buster-slim AS intermediate

# Install dependencies
RUN apt-get -qq update; \
    apt-get install -qqy --no-install-recommends \
        gnupg2 wget ca-certificates apt-transport-https \
        autoconf automake cmake dpkg-dev file make patch libc6-dev cmake git \
        fakeroot build-essential ncurses-dev xz-utils libssl-dev bc flex \
        libelf-dev bison python3 python3-pip cpio

# Install LLVM
RUN echo "deb https://apt.llvm.org/buster llvm-toolchain-buster-10 main" \
        > /etc/apt/sources.list.d/llvm.list && \
    wget -qO /etc/apt/trusted.gpg.d/llvm.asc \
        https://apt.llvm.org/llvm-snapshot.gpg.key && \
    apt-get -qq update && \
    apt-get install -qqy -t llvm-toolchain-buster-10 clang-10 \
    clang-tidy-10 clang-format-10 libc++-10-dev libc++abi-10-dev && \
    for f in /usr/lib/llvm-*/bin/*; do ln -sf "$f" /usr/bin; done && \
    ln -sf clang /usr/bin/cc && \
    ln -sf clang /usr/bin/c89 && \
    ln -sf clang /usr/bin/c99 && \
    ln -sf clang++ /usr/bin/c++ && \
    ln -sf clang++ /usr/bin/g++ && \
    rm -rf /var/lib/apt/lists/*

# Create Util dirs
WORKDIR /tmp/log
ADD tests /tmp/tests

# Build FiTx
ARG project_path
ADD src ${project_path}/src
ADD scripts ${project_path}/scripts
WORKDIR ${project_path}/build
RUN cmake -S ${project_path}/src && make -j ${nproc}
RUN pip3 install -r ${project_path}/scripts/requirements.txt

# Download and configure Linux Kernel
ARG linux_path
ARG linux_tar=linux.tar.gz
WORKDIR ${linux_path}
RUN wget https://cdn.kernel.org/pub/linux/kernel/v5.x/linux-5.15.1.tar.gz \
        -O ${linux_tar} && \
    tar xvf ${linux_tar} && \
    mv linux*/* . && \
    rm ${linux_tar}
# RUN git clone https://github.com/torvalds/linux.git ${linux_path}
# RUN git checkout v5.15 && \
RUN cp ${project_path}/scripts/config .config && \
    make CC=clang HOSTCC=clang olddefconfig

FROM intermediate as final
