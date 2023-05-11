#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include "common.h"




static int run_flag = 1;


// 接收客户端线程
void* thread_recv(void* arg)
{
    pthread_detach(pthread_self());
    int fd = *(int *)arg;
    char buffer[MAX_BUFFER_SIZE]={0};
    ssize_t numbytes = 0;
 
    while(run_flag)
    {
        numbytes = event_recv(fd, buffer, MAX_BUFFER_SIZE);
        if(numbytes <= 0)
        {
            printf("TCP server fd:(%d) client exit!\n", fd);
            goto EXIT;
        }
        //printf("fd:(%d) Received message: %s\n",fd, buffer);

        // 发送数据
        event_send(fd, buffer, strlen(buffer));
        memset(buffer, 0, sizeof(buffer)); 
    }

EXIT:
    close(fd);
    return (void*)(-1);
}

static int getAddress(int sockfd, int localfd, struct sockaddr_in client_sockaddr)
{
    struct sockaddr_in local_address;
    socklen_t address_length = sizeof(local_address);
    
    if (getsockname(sockfd, (struct sockaddr*)&local_address, &address_length) == 0)
    {
        char* address_p = inet_ntoa(local_address.sin_addr);
        fprintf(stdout, "LocalIP:%s, LocalPort:%hu\n", address_p, SERVER_PORT);
    } 
    fprintf(stdout, "RemoteIP:%s, RemotePort:%hu\n", inet_ntoa(client_sockaddr.sin_addr), ntohs(client_sockaddr.sin_port));
    event_send(sockfd, CONNECT_SUCCESS, strlen(CONNECT_SUCCESS));
   
    return 0;
}

static int tcpServer() 
{
    int server_fd, client_fd;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // 创建套接字
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // 设置套接字选项
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
    }

    // 绑定地址和端口号
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(SERVER_PORT);
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // 监听端口
    if (listen(server_fd, 3) < 0) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }
    fprintf(stdout, "TCP server bind listening ...\n");

    // 等待连接
    while (run_flag) 
    {
        client_fd = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
        if (client_fd < 0) {
            perror("accept failed");
            exit(EXIT_FAILURE);
        }

        // 获取连接地址和本地地址
        getAddress(client_fd, server_fd, address);

        // 建立线程接收客户端数据
        // 可以在此处限制连接的客户端个数
        pthread_t tid;
        pthread_create(&tid, NULL, thread_recv, &client_fd);
    }
    
    close(client_fd);
    return 0;
}

// 客户端断掉，服务端收到长度为0的消息
// 服务端断掉，客户端收到长度为0的消息
int main() 
{
    tcpServer();

    return 0;
}
