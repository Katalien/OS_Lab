mkdir build
cd build
cmake ..
make
cd ..
cp ./build/Daemon .
rm -r build