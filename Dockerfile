FROM ubuntu:16.10

RUN apt-get update && apt-get install -y build-essential \
    sudo \
    cmake \
    cppcheck \
    clang-3.8 \
    ninja-build \
    wget \
    git \
    vim \
    freeglut3-dev \
    libsdl2-dev \
    libassimp-dev \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*

RUN mkdir mgfx \
    && git clone https://github.com/cmaughan/mgfx mgfx \
    && cd mgfx \
    && mkdir build \
    && cd build \
    && cmake .. \
    && cmake --build . \
    && ctest --verbose


