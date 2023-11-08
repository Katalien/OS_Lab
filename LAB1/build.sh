#!/bin/bash
pidPath="/var/run/daemon.pid"
[[ -f $pidPath ]] || sudo touch "$pidPath"
sudo chmod 666 "$pidPath"

mkdir build
cd build
cmake ..
make
cd ..
cp ./build/Daemon .
rm -r build

sudo chmod +x ./Daemon