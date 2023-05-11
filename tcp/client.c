#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include "common.h"





static int fd = 0;


int tcpClient() 
{
    int client_fd = -1;
    struct sockaddr_in serv_addr;
    char buffer[MAX_BUFFER_SIZE] = {0};

    // 创建套接字
    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }

    // 设置服务器地址和端口号
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERVER_PORT);
    serv_addr.sin_addr.s_addr = inet_addr(SERVER_ADDRESS);
    bzero(&(serv_addr.sin_zero),sizeof(serv_addr.sin_zero));
   
    // 连接服务器
    if (connect(client_fd, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr_in)) < 0) {
        printf("\nConnection Failed \n");
        goto EXIT;
    }

    fd = client_fd;
    // 发送数据
    while (1)
    {
        bzero(buffer, MAX_BUFFER_SIZE); 
        ssize_t numbytes = event_recv(client_fd, buffer, MAX_BUFFER_SIZE);
        
        if (numbytes <= 0)
        {  
            fprintf(stderr,"TCP server close!\n");
            goto EXIT;
        } 

        buffer[numbytes] = '\0'; 
        //printf("fd:(%d) Received message: %s\n", client_fd, buffer);
    }

EXIT:
    fprintf(stdout, "TCP client exit\n");
    close(client_fd);
    fd = -1; 
    return 0;
}

// client thread
void* thread_handle_Client(void* arg)
{
    pthread_detach(pthread_self());
    tcpClient();
}

int main()
{
    pthread_t thread_id;
    pthread_create(&thread_id, NULL, thread_handle_Client, NULL);
    while(fd >= 0)
    {
        char msg[100];
        int len = 0;
        scanf("%s",msg); 
        len = strlen(msg);
        //sent to the server
        event_send(fd, (const void *)msg, len);
    }

    return 0;
}