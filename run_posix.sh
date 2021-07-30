#!/bin/sh

echo "Building Posix target..."

rm CMakeCache.txt
cmake .
make

echo "Done!"
