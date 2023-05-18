#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#define SERVER_PORT 8080
#define CERT_FILE "/mnt/hgfs/SwapTmp/epoll-master/curl/demo1/server.crt"
#define KEY_FILE "/mnt/hgfs/SwapTmp/epoll-master/curl/demo1/server.key"

int main() {
    SSL_CTX *ctx;
    SSL *ssl;
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len;
    
    // 初始化SSL库
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
    
    // 创建SSL上下文
    ctx = SSL_CTX_new(SSLv23_server_method());
    
    // 加载证书和私钥
    if (SSL_CTX_use_certificate_file(ctx, CERT_FILE, SSL_FILETYPE_PEM) <= 0) {
        perror("Error loading certificate file");
        exit(EXIT_FAILURE);
    }
    
    if (SSL_CTX_use_PrivateKey_file(ctx, KEY_FILE, SSL_FILETYPE_PEM) <= 0) {
        perror("Error loading private key file");
        exit(EXIT_FAILURE);
    }
    
    // 创建TCP套接字
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }
    
    // 设置服务器地址和端口
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(SERVER_PORT);
    
    // 绑定套接字到服务器地址
    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("Error binding socket");
        exit(EXIT_FAILURE);
    }
    
    // 监听连接
    if (listen(server_fd, 10) == -1) {
        perror("Error listening on socket");
        exit(EXIT_FAILURE);
    }
    
    printf("Server listening on port %d\n", SERVER_PORT);
    
    while (1) {
        // 等待客户端连接
        client_len = sizeof(client_addr);
        client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
        if (client_fd == -1) {
            perror("Error accepting connection");
            exit(EXIT_FAILURE);
        }
        
        printf("Client connected: %s\n", inet_ntoa(client_addr.sin_addr));
        
        // 创建SSL连接
        ssl = SSL_new(ctx);
        SSL_set_fd(ssl, client_fd);
        
        // SSL握手
        if (SSL_accept(ssl) <= 0) {
            perror("Error performing SSL handshake");
            exit(EXIT_FAILURE);
        }
        
        printf("SSL handshake completed\n");
        
        // 在SSL连接上发送和接收数据
        char buffer[1024];
        memset(buffer, 0, sizeof(buffer));
        SSL_read(ssl, buffer, sizeof(buffer)-1);
        printf("Received from client: %s\n", buffer);
        
        const char* response = "Hello, client!";
        SSL_write(ssl, response, strlen(response));
        printf("Sent:%s\n", response);


        // 关闭SSL连接
        SSL_shutdown(ssl);
        SSL_free(ssl);
        
        // 关闭客户端套接字
        close(client_fd);
    }

    // 清理并释放资源
    SSL_CTX_free(ctx);
    close(server_fd);

    return 0;
}