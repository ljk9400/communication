#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/select.h>
#include "../tcp/common.h"




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
    if (listen(server_fd, 5) < 0) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }
    fprintf(stdout, "TCP server bind listening ...\n");

   
    // Select
    fd_set read_fds, r_fds;  // 这里一定要二个fd_set，不然bug调试不出来，猜测selet和FD_SET
    FD_ZERO(&read_fds);
    FD_SET(server_fd, &read_fds);
    int max_fd = server_fd, client_s_fd[MAX_CONNECT_CNT];

    for (int i = 0; i < MAX_CONNECT_CNT; i++) {
        client_s_fd[i] = -1;
    }


    // 等待连接
    while (run_flag) 
    {
        r_fds = read_fds;
        if (select(max_fd + 1, &r_fds, NULL, NULL, NULL) < 0) {
            perror("select");
            exit(EXIT_FAILURE);
        }

        // 处理客户端连接
        if (FD_ISSET(server_fd, &r_fds)) 
        {
            client_fd = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
            if (client_fd < 0) {
                perror("accept failed");
                exit(EXIT_FAILURE);
            }

            // 获取连接地址和本地地址
            getAddress(client_fd, server_fd, address);
            
            // add new client file descriptor to array
            int isMaxConnect = 1;
            for (int i = 0; i < MAX_CONNECT_CNT; i++) {
                if (client_s_fd[i] == -1) {
                    client_s_fd[i] = client_fd;
                    isMaxConnect = 0;
                    break;
                }
            }

            // add new client file descriptor to file descriptor set
             if(isMaxConnect){
                printf("TCP server fd:(%d) client close!\n", client_fd);
                close(client_fd);
            }
            else{
                FD_SET(client_fd, &read_fds);
                if (client_fd > max_fd) {
                    max_fd = client_fd;
                }
            }
           
            // for (int i = 0; i < MAX_CONNECT_CNT; i++) {
            //     printf("fd %d max %d %x\n", client_s_fd[i], max_fd, read_fds.__fds_bits[0]);
            // }
        }

        // 处理客户端消息
        for (int i = 0; i < MAX_CONNECT_CNT; i++) 
        {
            int fd = client_s_fd[i];
            if (fd != -1 && FD_ISSET(fd, &r_fds)) {
                char buffer[MAX_BUFFER_SIZE]={0};
                ssize_t numbytes = event_recv(fd, buffer, MAX_BUFFER_SIZE);
                if (numbytes <= 0) 
                {
                    printf("TCP server fd:(%d) client exit!\n", fd);
                    FD_CLR(fd, &read_fds);
                    close(fd);
                    if (fd == max_fd)
                    {
                        client_s_fd[i] = -1;
                        max_fd = server_fd;
                        for (int i = 1; i < MAX_CONNECT_CNT; i++){
                            if (client_s_fd[i] > max_fd)
                                max_fd = client_s_fd[i];
                        } 
                    }
                    client_s_fd[i] = -1;
                } 
                else {
                    // 发送数据
                    event_send(fd, buffer, strlen(buffer));
                }
            }
        }
    }
    
    close(server_fd);
    return 0;
}

// 客户端断掉，服务端收到长度为0的消息
// 服务端断掉，客户端收到长度为0的消息
int main() 
{
    tcpServer();
    return 0;
}
