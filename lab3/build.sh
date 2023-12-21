#!/bin/bash

mkdir -p build
cd build || exit
cmake ..
make
cp Lab3 ..
cd ..
rm -rf build
