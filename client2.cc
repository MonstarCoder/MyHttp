#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <string.h>
#include <pthread.h>

void* thread_func(void* args);

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        printf("usage: a.out <IP>\n");
        exit(-1);
    }
    
    // pthread_attr_t pthread_attr_detach;
    // pthread_attr_init(&pthread_attr_detach);
    // pthread_attr_setdetachstate(&pthread_attr_detach, PTHREAD_CREATE_DETACHED);

    for (int i = 0; i < 1024; ++i) 
    {
        for (int j = 0; j < 100; ++j)
        {
            pthread_t tid;
            // pthread_create(&tid, &pthread_attr_detach, &thread_func, (void*)argv[1]);
            // pthread_create(&tid, NULL, &thread_func, (void*)argv[1]);
            printf("NO.%d is running\n", 1024 * i + j);
            thread_func((void*)argv[1]);
        }
        // sleep(2);
    }

    return 0;

}

void* thread_func(void* args)
{
    int sockfd;
    struct sockaddr_in addr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(80);
    char* addr_char = (char*)(args);
    addr.sin_addr.s_addr = inet_addr(addr_char);

    int len = sizeof(addr);
    if ((connect(sockfd, (struct sockaddr*)&addr, len)) == -1)
    {
        perror("connect error");
        exit(-1);
    }
    else
    {
        printf("connect success\n");
    }

	char http_request[] =
/*"GET /home/marvin/MyHttp/index.html HTTP2.1\r\n\*/
"GET /root/MyHttp/index.html HTTP1.1\r\n\
Lengh: 8080\r\n\
Date: Fri Mon 2018\r\n\
\r\n\
<html>\n\
sb\n\
</html>"
    ;

    if (write(sockfd, &http_request, sizeof(http_request)) < 0)
    {
        perror("write error");
        exit(-1);
    }
    printf("write success\n");
    char *read_buff = new char[10240];
    int in = 0, nread = 0;
    while((in = read(sockfd, read_buff + nread, sizeof(read_buff))) > 0)
    {
        nread += in;
        if (nread > 10000)
            break;
    }
    printf("read from server:\n%s", read_buff);
    delete[] read_buff;
    close(sockfd);
    return NULL;
}