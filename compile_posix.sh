#!/bin/sh

echo "Free RTOS injection simulator: building for Posix target..."

rm -f CMakeCache.txt

echo "Compiling..."

cmake . &> cmake_build.log
if [[ $? -ne 0 ]]; then
    echo "cmake error! Check cmake_build.log"
    exit 1
fi

make &> make_build.log
if [[ $? -ne 0 ]]; then
    echo "make compilation error! Check cmake_build.log"
    exit 2
fi

cp build/sim sim.out

echo "Done! You can run the simulator with ./sim.out"
