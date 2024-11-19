#!/bin/bash

source /etc/profile

source /etc/profile

build_folder=$PWD/build

rm -rf $build_folder


mkdir $build_folder


pushd $build_folder;

cmake -DCMAKE_TOOLCHAIN_FILE=./config.cmake .. ;

make -j;

popd ;