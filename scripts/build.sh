#!/bin/bash
set -e

CXX_FLAGS="-O3 -march=native -flto -DNDEBUG"

cmake -B build \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_CXX_FLAGS_RELEASE="$CXX_FLAGS"

cmake --build build -j$(nproc)
