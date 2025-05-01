#!/bin/bash

#bash script to make building main.cpp faster

rm -rf build
mkdir build
cd build
cmake ..
if make; then
    ./StrategyGame
else
    echo "Build failed, skipping execution"
    exit 1
fi