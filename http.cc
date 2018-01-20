#include "http.h"

// 处理客户端链接的线程例程
void* thread_func(void* param);

// 记录当前处理线程的数量
int32_t thread_num = 0;
pthread_mutex_t thread_num_mutex = PTHREAD_MUTEX_INITIALIZER;

// thread_num原子加１
inline void thread_add1();

// thread_num原子减１
inline void thread_minus1();

// thread_num原子读
int32_t thread_num_read();

// HttpHeader处理函数，根据解析下来的HttpHeader来处理客户的请求
// result保存了处理结果，即http响应包
// 返回值：HTTP状态码
int do_http_header(HttpHeader* hh, string& result);

// 通过HTTP状态码返回友好语句
inline const char* get_state_by_codes(int http_codes);

//**********web服务器程序入口函数**********
int main(int argc, char**argv)
{
    if (argc != 2)
    {
        std::cout << "Usage: " << argv[0] << " <config_path>" << std::endl;
        exit(-1);
    }

    // p判断配置文件是否存在
    if (file_existed(argv[1]) == false)
    {
        perror("cannot open config file");
        exit(-1);
    }

    // 创建监听套接字
    int listen_fd = my_socket(AF_INET, SOCK_STREAM, 0);
    // 设置套接字为非阻塞模式
    set_nonblock(listen_fd);
    // 设置套接字的SO_REUSEADDR选项
    set_reuse_addr(listen_fd);
    // 通过服务名和协议名获得相应的知名端口
    // http端口为80,可直接设置
    struct servent *pservent = getservbyname("http", "tcp");
    // pservent->s_port已经是网络字节序了
    // uint16_t参见UNP 3.2
    uint16_t listen_port = pservent->s_port;

    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;

    // 设定套接字地址
    bzero(&server_addr, sizeof(server_addr)); 
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = listen_port;
    
    my_bind(listen_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    my_listen(listen_fd, MAX_BACKLOG);

    // 创建epoll文件描述符
    int epollfd = my_epoll_create(MAX_EVENTS);

    struct epoll_event ev;
    struct epoll_event events[MAX_EVENTS];

    ev.events = EPOLLIN; // 可读事件
    ev.data.fd = listen_fd;
    my_epoll_ctl(epollfd, EPOLL_CTL_ADD, listen_fd, &ev);

    // 设置线程属性为detach
    pthread_attr_t pthread_attr_detach;
    pthread_attr_init(&pthread_attr_detach);
    pthread_attr_setdetachstate(&pthread_attr_detach, PTHREAD_CREATE_DETACHED);

    int connfd;
    int nfds;
    socklen_t addrlen;
    pthread_t tid;
    Epollfd_Connfd epollfd_connfd;
    for (; ;)
    {
        // 不设置超时
        nfds = my_epoll_wait(epollfd, events, MAX_EVENTS, -1);
        // 若my_epoll_wait被中断则重新调用该函数
        if (nfds == -1 &&  errno == EINTR)
            continue;

        for (int i = 0; i != nfds; ++i)
        {
            // 处理监听套接字触发的事件
            if (events[i].data.fd == listen_fd)
            {
                connfd = my_accept(listen_fd, (struct sockaddr*)&client_addr, &addrlen);
                if (connfd < 0)
                {
                    perror("connfd < 0");
                    exit(-1);
                }
                // 设置新连接上的套接字为非阻塞模式
                set_nonblock(connfd);
                // 设置读事件和ET模式
                ev.events = EPOLLIN | EPOLLET;
                ev.data.fd == connfd;
                // 将监听事件加入epoll中
                my_epoll_ctl(epollfd, EPOLL_CTL_ADD, connfd, &ev);
            }
            // 处理已连接的用户
            else
            {
                epollfd_connfd.epollfd = epollfd;
                epollfd_connfd.connfd = events[i].data.fd;
                ev.data.fd = connfd;
                // epoll不再监听这个客户端套接字
                my_epoll_ctl(epollfd, EPOLL_CTL_DEL, connfd, &ev);
                // 处理链接
                pthread_create(&tid, &pthread_attr_detach, &thread_func, (void*)&epollfd_connfd);
            }
        }
    }
    // 清理工作
    pthread_attr_destroy(&pthread_attr_detach);

    // 关闭监听套接字
    close(listen_fd);
    
    return 0;
}

// 处理客户端链接的线程例程
#define TIMEOUT 1000 * 60 * 4 // 设置超时　milliseconds
void* thread_func(void* param)
{
    thread_num_add1();

    HttpHeader* hh = alloc_http_header();

    Epollfd_Connfd* ptr_epollfd_connfd = (Epollfd_Connfd*)param;

    // 获取客户链接的socket
    int connfd = ptr_epollfd_connfd->connfd;

    struct epoll_event ev, events[2];
    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = connfd;
    int epollfd = my_epoll_create(2);
    my_epoll_ctl(epollfd, EPOLL_CTL_ADD, ev.data.fd, &ev);
    int nfds = 0;

    // 设置http请求包大小
    const int ONEKILO = 1024;
    constexpr int ONEMEGA = 1024 * ONEKILO;
    constexpr int ONEGIGA = 1024 * ONEMEGA;
    char* buff = (char*)my_malloc(ONEMEGA);
    bzero(buff, ONEMEGA);

    // 关闭nagle
    set_off_tcp_nagle(connfd);
    // 设置接收超时时间为60秒
    set_recv_timeo(connfd, 60, 0);
    // 设置发送时间为120秒
    // set_send_timeo(connfd, 120, 0);

//TODO


}