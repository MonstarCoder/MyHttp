#ifndef MYHTTP_UTILITY_H_
#define MYHTTP_UTILITY_H_

#include <sys/socket.h>
#include <sys/types.h>
#include <stdlib.h>
#include <netdb.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/epoll.h>
#include <string.h>
#include <sys/stat.h>
#include <netinet/tcp.h>
#include <strings.h>

#include <fstream>
#include <sstream>
#include <string>
#include <iostream>
#include <map>

using std::string;

// 配置文件信息
#define DOMAIN 2
#define DOCROOT 1
extern string domain;
extern string docroot;

// 返回GMT时间
string gmt_time();

// 根据http请求中的url和配置文件中的docroot配置选项构造真正的url
string real_url(const string& url);

// 检查文件是否存在
inline bool file_existed(const char* path);

// 获取文件长度
int get_file_length(const char* path);

// 获取文件最后修改时间
string get_file_modified_time(const char* path);

// 初始化全局变量config_keyword_map
// 必须在使用config_keyword_map前调用此函数
void init_config_keyword_map();

// 解析配置文件
bool parse_config(const char* path);

// 设置文件描述符为非阻塞模式
void set_nonblock(int fd);

// 设置套接字SO_REUSEADDR选项
void set_reuse_addr(int sockfd);

// 设置套接字TCP_NODELAY选项，关闭nagle算法
void set_off_tcp_nagle(int sockfd);

// 开启nagle
void set_on_tcp_nagle(int sockfd);

// 开启套接字TCP_CORK选项
// CORK选项提高了网络利用率，因为它直接禁止了小包的发送，而naggle没有这么做
void set_on_tcp_cork(int sockfd);

// 关闭TCP_CORK
void set_off_tcp_cork(int sockfd);

// 设置套接字SO_RCVTIMEO选项，接收超时
// 函数参数：sec秒，msec微妙
void set_recv_timeo(int sockfd, int sec, int msec);

// 设置套接字SO_SNDTIMEO选项，发送超时
void set_send_timeo(int sockfd, int sec, int msec);

//**********系统函数的包装**********
int my_socket(int domain, int type, int protocol);
void my_bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
void my_listen(int sockfd, int backlog);
int my_accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
struct servent* my_getservbyname(const char *name, const char *proto);
int my_epoll_create(int size);
void my_epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);
int my_epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout);
void *my_calloc(size_t nmemb, size_t size);
void *my_malloc(size_t size);
void my_free(void *ptr);

#endif