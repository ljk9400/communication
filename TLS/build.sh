
#!/bin/bash

build_curl() {
    echo "build curl"
    rm test_curl.o
    gcc curl.c -g -o test_curl.o -L ./ -lcurl  -lpthread
}

build_server() {
    echo "build server"
    rm test_server.o
    gcc server.c -g -o test_server.o -lssl -lcrypto  -lpthread
}

build_epoll() {
    echo "build server"
    rm test_server.o
    gcc epoll.c -g -o test_server.o -lssl -lcrypto  -lpthread
}


run_curl() {
    echo "run curl"
    export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:./
    ./test_curl.o 
}

run_server() {
    echo "run server"
    export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:./
    ./test_server.o 
}



if [ $# = 0 ]; then
    echo "nothing"
else
    if [ $1 == 'c' ]; then
        build_curl
    elif [ $1 == 's' ]; then
        build_server
    elif [ $1 == 'e' ]; then
        build_epoll
    elif [ $1 == 'run-c' ];then
        run_curl
    elif [ $1 == 'run-s' ];then
        run_server
    fi
fi