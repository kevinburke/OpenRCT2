#!/bin/bash

set -e

mkdir -p build
pushd build 
    cmake -DCMAKE_TOOLCHAIN_FILE=../CMakeLists_mingw.txt -DCMAKE_BUILD_TYPE=Debug  ..
    make
popd

