FROM ubuntu:22.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    g++ \
    pkg-config \
    git \
    curl \
    wget \
    7zip \
    p7zip-full \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY CMakeLists.txt .
COPY src/ src/
COPY include/ include/
COPY tests/ tests/

RUN mkdir -p build && cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release -DFML_BUILD_CLI=ON -DFML_BUILD_TESTS=ON && \
    make -j$(nproc)

FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    libstdc++6 \
    libgcc-s1 \
    curl \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY --from=builder /app/build/src/cli/fivem-linux /usr/local/bin/fivem-linux
COPY --from=builder /app/build/libfivemlinux.so.1.0.0 /usr/local/lib/libfivemlinux.so
COPY --from=builder /app/build/libfivemlinux.so.1 /usr/local/lib/
COPY --from=builder /app/build/libfivemlinux.so /usr/local/lib/

RUN ldconfig

CMD ["fivem-linux", "--help"]
