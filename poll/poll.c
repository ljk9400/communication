#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <poll.h>
#include "../tcp/common.h"



static int run_flag = 1;


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

    // 设置服务器地址
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(SERVER_PORT);
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // 监听端口
    if (listen(server_fd, 5) < 0) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }
    fprintf(stdout, "TCP server bind listening ...\n");


    // poll, 也可建立线程将检测单独出去
    struct pollfd pfd[MAX_CONNECT_CNT];
    pfd[0].fd = server_fd;
    pfd[0].events = POLLIN;
    int max_fd = server_fd;
    for (int i = 1; i < MAX_CONNECT_CNT; i++){
        pfd[i].fd = -1;
    }

    // 等待连接
    while (run_flag) 
    {
        poll(pfd, max_fd+1, -1);

        // 服务端接收事件
        if(pfd[0].revents & POLLIN)
        {
            client_fd = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
            if (client_fd < 0) {
                perror("accept failed");
                exit(EXIT_FAILURE);
            }
            // 获取连接地址和本地地址
            getAddress(client_fd, server_fd, address);

            int isMaxConnect = 1;
            for (int i = 1; i < MAX_CONNECT_CNT; i++){
                if (pfd[i].fd < 0){
                    pfd[i].fd = client_fd;
                    pfd[i].events = POLLIN;
                    isMaxConnect = 0;
                    break;
                }
            }

            // 最大连接限制
            if(isMaxConnect){
                printf("TCP server fd:(%d) client close!\n", client_fd);
                close(client_fd);
            }
            else{
                if (client_fd > max_fd)
                    max_fd = client_fd;
            }
            
            // printf("max_fd %d\n",max_fd);
            // for (int i = 0; i < MAX_CONNECT_CNT; i++){
            //    printf("fd %d\n",pfd[i].fd);
            // }
        }
       
        // 客户端接收事件
        for (int i = 1; i < MAX_CONNECT_CNT; i++)
        {
            if (pfd[i].fd > 0 && (pfd[i].revents & POLLIN))
            {
                char buffer[MAX_BUFFER_SIZE]={0};
                ssize_t numbytes = event_recv(pfd[i].fd, buffer, MAX_BUFFER_SIZE);
                if(numbytes <= 0)
                {
                    printf("TCP server fd:(%d) client exit!\n", pfd[i].fd);
                    close(pfd[i].fd);
                    if (pfd[i].fd == max_fd)
                    {
                        pfd[i].fd = -1;
                        max_fd = server_fd;
                        for (int i = 1; i < MAX_CONNECT_CNT; i++){
                            if (pfd[i].fd > max_fd)
                                max_fd = pfd[i].fd;
                        } 
                    }
                    pfd[i].fd = -1;
                }
                else
                {
                    // 发送数据
                    event_send(pfd[i].fd, buffer, strlen(buffer));
                }
            }
        } 
    }
    
    close(server_fd);
    return 0;
}

// 服务端断掉，客户端收到长度为0的消息
int main() 
{
    tcpServer();

    return 0;
}
