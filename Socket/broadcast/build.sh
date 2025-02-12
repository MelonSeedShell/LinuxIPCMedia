#!/bin/bash

g++ -o svr ./src/BroadcastServer.cpp ./src/server_main.cpp -I ./include/ -lpthread