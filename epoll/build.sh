#!/bin/bash


build_epoll() {
    echo "build epoll"
    rm test_epoll.o
    gcc epoll.c  -g -o test_epoll.o  -lpthread
}

build_client() {
    echo "build client"
    rm test_client.o
    gcc client.c  -g -o test_client.o  -lpthread
}

run_epoll() {
    echo "run epoll"
    export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:./
    ./test_epoll.o 
}

run_client() {
    echo "run client"
    export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:./
    ./test_client.o 
}

if [ $# = 0 ]; then
    echo "no thing"
else
    if [ $1 == 'ep' ]; then
        build_epoll
    elif [ $1 == 'c' ];then
        cd ../tcp
        build_client
    elif [ $1 == 'run-ep' ];then
        run_epoll
    elif [ $1 == 'run-c' ];then
        cd ../tcp
        run_client
    fi
fi