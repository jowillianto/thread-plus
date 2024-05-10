FROM ubuntu:22.04

# Install Dependencies
RUN apt-get update && apt install -y \
  git \
  curl \
  wget \
  make \
  build-essential \
  software-properties-common \
  gnupg \
  python3
# Install cmake
WORKDIR /opt
ARG TARGETPLATFORM
ENV TARGETPLATFORM $TARGETPLATFORM
RUN bash -c '[ "$TARGETPLATFORM" == "linux/arm64" ] && \
  curl -sfSL \
  https://github.com/Kitware/CMake/releases/download/v3.28.4/cmake-3.28.4-linux-aarch64.sh -o \
  install_cmake.sh \
  || \
  curl -sfSL \
  https://github.com/Kitware/CMake/releases/download/v3.29.2/cmake-3.29.2-linux-x86_64.sh -o \
  install_cmake.sh'
RUN chmod +x install_cmake.sh
RUN ./install_cmake.sh --skip-license --prefix=/usr
RUN cmake --version

# Install ninja
WORKDIR /opt
RUN git clone https://github.com/ninja-build/ninja
WORKDIR /opt/ninja
RUN git checkout release
RUN CXX=g++ CC=gcc cmake -B build -DCMAKE_BUILD_TYPE=Release .
RUN ./configure.py --bootstrap
RUN mv ninja /usr/bin/ninja

# Install llvm
WORKDIR /opt
RUN curl -sfSL https://apt.llvm.org/llvm.sh -o llvm.sh
RUN chmod +x llvm.sh
RUN ./llvm.sh 18 all
RUN apt clean

WORKDIR /app
COPY . .
RUN CXX=clang++-18 cmake -B build -DCMAKE_BUILD_TYPE=Release -GNinja .
WORKDIR /app/build
RUN ninja thread_plus_tests
RUN ./thread_plus_tests