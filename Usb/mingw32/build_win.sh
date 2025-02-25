#!/bin/bash

x86_64-w64-mingw32-g++ -o usb.exe usb.cpp -lsetupapi -luuid -std=c++11 -static -lstdc++ -L./.libusb/lib -I./.libusb/include -lusb-1.0 

# mkdir -p build

# cd build
# cmake ..
# make
# cd -
