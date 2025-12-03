#!/bin/bash

export THIRDPARTY="/workspace"

sudo rm -rf build
mkdir build
cd build
cmake .. && make

