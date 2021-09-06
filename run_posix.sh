#!/bin/sh

echo "Building Posix target..."

rm -f CMakeCache.txt
cmake .
make

echo "Done!"
