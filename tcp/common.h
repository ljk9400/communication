#ifndef UTILITY_H_INCLUDED
#define UTILITY_H_INCLUDED

#include <sys/socket.h>
#include <stdio.h>
#include <errno.h>


#define MAX_CONNECT_CNT      (5)
#define SERVER_ADDRESS       "192.168.20.134"
#define SERVER_PORT          (8080)
#define MAX_BUFFER_SIZE      (1024)
#define CONNECT_SUCCESS     "You have conected the server!"

// data transmission
static ssize_t event_send(int sockFd, const char* buff, int len)
{
    ssize_t ret = send(sockFd, (const void *)buff, len, 0);
    fprintf(stdout,"TCP fd:(%d) >> %s, len:%d, ret:%zd\n",sockFd, buff, len, ret);
    if(ret == -1)
    {
        fprintf(stderr,"send errno %d!\n", errno);
    }
    return ret;
}

// Data receiving
static ssize_t event_recv(int sockFd, char* buff, int len)
{
    ssize_t ret = recv(sockFd, (void *)buff, len, 0);
    fprintf(stdout,"TCP fd:(%d) << %s, len:%zd\n",sockFd, buff, ret);
   
    if(ret == -1)
    {
        fprintf(stderr,"recv errno %d!\n", errno);
    }
    return ret;
}

#endif