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

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        printf("usage: a.out <IP>\n");
        exit(-1);
    }
    int sockfd;
    int len;
    struct sockaddr_in addr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(80);
    addr.sin_addr.s_addr = inet_addr(argv[1]);
    len = sizeof(addr);
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
"GET /root/MyHttp/Google.html HTTP1.1\r\n\
Lengh: 8080\r\n\
Date: Fri Mon 2018\r\n\
\r\n\
<html>\n\
sb\n\
</html>"
    ;
    char buf[1024];
    if (write(sockfd, &http_request, sizeof(http_request)) < 0)
    {
        perror("write error");
        exit(-1);
    }
    printf("write success");
    if (read(sockfd, &buf, sizeof(buf)) < 0)
    {
        perror("read error");
    }
    printf("read from server:%s", buf);
    close(sockfd);
    exit(0);
}