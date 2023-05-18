#include <stdio.h>
#include <curl/curl.h>

struct MemoryStruct {
  char *memory;
  size_t size;
};

static size_t writeFunctionCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
  size_t realsize = size * nmemb;
  struct MemoryStruct *mem = (struct MemoryStruct *)userp;
  char *ptr = realloc(mem->memory, mem->size + realsize + 1);
  if(ptr == NULL)return 0;
  mem->memory = ptr;
  memcpy(&(mem->memory[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->memory[mem->size] = 0;
  return realsize;
}

static size_t headerFunctionCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
  size_t realsize = size * nmemb;
  struct MemoryStruct *mem = (struct MemoryStruct *)userp;
  char *ptr = realloc(mem->memory, mem->size + realsize + 1);
  if(ptr == NULL)return 0;
  mem->memory = ptr;
  memcpy(&(mem->memory[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->memory[mem->size] = 0;
  return realsize;
}

int main() {
    CURL *curl;
    CURLcode res;

    char errbuf[CURL_ERROR_SIZE]={0};  //256
    
    struct MemoryStruct chunkwirte;
    struct MemoryStruct chunkheader;
    chunkwirte.memory = malloc(1);  /* will be grown as needed by the realloc above */ 
    chunkwirte.size = 0;    /* no data at this point */	
    
    chunkheader.memory = malloc(1);  /* will be grown as needed by the realloc above */ 
    chunkheader.size = 0;    /* no data at this point */
    
    // 初始化CURL句柄
    curl = curl_easy_init();
    if (curl) {
        // 设置要访问的URL
        //curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:8080/api/data");

        curl_easy_setopt(curl, CURLOPT_NOBODY,         0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L); //验证证书
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L); //检查证书中的公用名是否存在，并且是否与提供的主机名匹配
        
        //curl_easy_setopt(curl, CURLOPT_CAPATH, "/mnt/hgfs/SwapTmp/epoll-master/curl/demo1/certs/ca.pem"); //指定证书路径
        //curl_easy_setopt(curl, CURLOPT_CAINFO, "/mnt/hgfs/SwapTmp/epoll-master/curl/demo1/certs/ca.pem"); //指定证书信息
        curl_easy_setopt(curl, CURLOPT_CAPATH, "/mnt/hgfs/SwapTmp/epoll-master/curl/demo1/server.crt"); //指定证书路径
        curl_easy_setopt(curl, CURLOPT_CAINFO, "/mnt/hgfs/SwapTmp/epoll-master/curl/demo1/server.crt"); //指定证书信息
        //curl_easy_setopt(curl, CURLOPT_CAPATH,  "/etc/ssl/certs/");//指定证书信息
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10 );//接收数据时超时设置，如果10秒内数据未接收完，直接退出
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10);//连接超时，这个数值如果设置太短可能导致数据请求不到就断开了
        curl_easy_setopt(curl, CURLOPT_FORBID_REUSE, 0); //默认是7200S
        curl_easy_setopt(curl, CURLOPT_MAXCONNECTS,  20); //默认是5
        //curl_easy_setopt(_curl->curl, CURLOPT_DNS_CACHE_TIMEOUT , 240); //默认是7200S
        curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
        curl_easy_setopt(curl, CURLOPT_VERBOSE,  1); //curl日志打印，0关闭，1打开


        curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errbuf);
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "GET");
        //curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST,   "POST");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,  writeFunctionCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA,      (void *)&chunkwirte);
        curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, headerFunctionCallback);
        curl_easy_setopt(curl, CURLOPT_HEADERDATA,     (void *)&chunkheader);
        curl_easy_setopt(curl, CURLOPT_ERRORBUFFER,     errbuf);//请求返回值不是CURLE_OK,则将错误码的信息存放到errbuf中
//curl_easy_setopt(curl, CURLOPT_URL, "https://123.125.81.19/api/data");
curl_easy_setopt(curl, CURLOPT_URL, "https://127.0.0.1/api/data");
curl_easy_setopt(curl, CURLOPT_PORT, 8080); // 设置自定义的 HTTPS 端口
        
        

        // 创建并设置头部信息
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        headers = curl_slist_append(headers, "Authorization: Bearer token123");
curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        // 设置请求主体
        //const char *body = "{\"key\": \"value\"}";
        const char *body = "1234444";
        //curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, 100);
curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body);

   
    
    

        // 执行HTTP请求
        errbuf[0]='\0';
        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        }
          
    printf("heloo\n");
    printf("head:%s, %d\n", chunkheader.memory, chunkheader.size);
    printf("body:%d, %s, %s, %d\n", res, errbuf, chunkwirte.memory, chunkwirte.size);

        // 释放头部信息内存
        curl_slist_free_all(headers);

        // 清理CURL句柄
        curl_easy_cleanup(curl);
    }
    
    return 0;
}
