#!/bin/bash


build_server() {
    echo "build server"
    rm test_server.o
    gcc server.c  -g -o test_server.o  -lpthread
}

build_client() {
    echo "build client"
    rm test_client.o
    gcc client.c  -g -o test_client.o  -lpthread
}

run_server() {
    echo "run server"
    export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:./
    ./test_server.o 
}

run_client() {
    echo "run client"
    export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:./
    ./test_client.o 
}

if [ $# = 0 ]; then
    echo "no thing"
else
    if [ $1 == 's' ]; then
        build_server
    elif [ $1 == 'c' ];then
        build_client
    elif [ $1 == 'run-s' ];then
        run_server
    elif [ $1 == 'run-c' ];then
        run_client
    fi
fi