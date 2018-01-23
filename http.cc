#include "http.h"

// 设置http请求包大小
#define ONEKILO 1024
#define ONEMEGA 1024 * ONEKILO
#define ONEGIGA 1024 * ONEMEGA

// 处理客户端链接的线程例程
void* thread_func(void* param);

// 记录当前处理线程的数量
int32_t thread_num = 0;
pthread_mutex_t thread_num_mutex = PTHREAD_MUTEX_INITIALIZER;

// thread_num原子加１
inline void thread_num_add1();

// thread_num原子减１
inline void thread_num_minus1();

// thread_num原子读
int32_t thread_num_read();

// thread_func辅助处理函数
void* thread_func_aux(HttpHeader* hh, EpollfdConnfd* ptr_epollfd_connfd);

// 清除函数
void clear(int confd, HttpHeader* buf);

// HttpHeader处理函数，根据解析下来的HttpHeader来处理客户的请求
// result保存了处理结果，即http响应包
// 返回值：HTTP状态码
int do_http_header(HttpHeader* hh, string& result);

// 通过HTTP状态码返回友好语句
inline const char* get_state_by_codes(int http_codes);

//**********web服务器程序入口函数**********
int main(int argc, char**argv)
{
    // if (argc != 2)
    // {
    //     std::cout << "Usage: " << argv[0] << " <config_path>" << std::endl;
    //     exit(-1);
    // }

    // // p判断配置文件是否存在
    // if (file_existed(argv[1]) == false)
    // {
    //     perror("cannot open config file");
    //     exit(-1);
    // }

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
    // pthread_attr_t pthread_attr_detach;
    // pthread_attr_init(&pthread_attr_detach);
    // pthread_attr_setdetachstate(&pthread_attr_detach, PTHREAD_CREATE_DETACHED);

    // 创建线程池
    Threads thread_pool(100);

    int connfd;
    int nfds;
    socklen_t addrlen;
    pthread_t tid;
    EpollfdConnfd epollfd_connfd;
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
                ev.data.fd = connfd;
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
                // pthread_create(&tid, &pthread_attr_detach, &thread_func, (void*)&epollfd_connfd);

                // 交给线程池中的一个线程
                thread_pool.add_task(&thread_func, (void*)&epollfd_connfd);
            }
        }
    }
    // 清理工作
    // pthread_attr_destroy(&pthread_attr_detach);

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

    EpollfdConnfd* ptr_epollfd_connfd = (EpollfdConnfd*)param;

    // 获取客户链接的socket
    int connfd = ptr_epollfd_connfd->connfd;

    struct epoll_event ev, events[2];
    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = connfd;
    int epollfd = my_epoll_create(2);
    my_epoll_ctl(epollfd, EPOLL_CTL_ADD, ev.data.fd, &ev);
    int nfds = 0;

    // 关闭nagle
    set_off_tcp_nagle(connfd);
    // 设置接收超时时间为60秒
    set_recv_timeo(connfd, 60, 0);
    // 设置发送时间为120秒
    set_send_timeo(connfd, 120, 0);

    // 处理http请求
    thread_func_aux(hh, ptr_epollfd_connfd);

    // nfds = my_epoll_wait(epollfd, events, 2, TIMEOUT);
    // if (nfds == 0) // 超时
    // {
    //     clear(connfd, hh);
    //     return NULL;
    // }
    // for (int i = 0; i< nfds; ++i)
    // {
    //     if (events[i].data.fd == connfd)
    //         thread_func_aux(hh, ptr_epollfd_connfd);
    //     else
    //     {
    //         clear(connfd, hh);
    //         return NULL;
    //     }
    // }
}

//  thread_func辅助处理函数
void* thread_func_aux(HttpHeader* hh, EpollfdConnfd* ptr_epollfd_connfd)
{
    int connfd = ptr_epollfd_connfd->connfd;
    int32_t nread = 0, n = 0;
    char* buff = (char*)my_malloc(ONEMEGA);
    bzero(buff, ONEMEGA);

    for ( ; ;)
    {
        if ((n = read(connfd, buff + nread, sizeof(*buff))) > 0)
            nread += n;
        else if (n == 0)
            break;
        else if (n == -1 && errno == EINTR)
            continue;
        else if (n == -1 && errno == EAGAIN)
            break;
        else if (n == -1 && errno == EWOULDBLOCK)
        {
            perror("socket read timeout");
            clear(connfd, hh);
            return NULL;
        }
        else
        {
            perror("read http request error");
            my_free(buff);
            break;
        }
    }

    if (nread != 0)
    {
        string str_http_request(buff, buff + nread);

        if (!parse_http_request(str_http_request, hh))
        {
            perror("parse_http_request failed");
            clear(connfd, hh);
            return NULL;
        }

        string out;
        int http_codes = do_http_header(hh, out);

        char* out_buf = (char*)my_malloc(out.size());
        if (out_buf == NULL)
        {
            clear(connfd, hh);
            return NULL;
        }
        
        int i;
        for (i = 0; i != out.size(); ++i)
            out_buf[i] = out[i];
        out_buf[i] = '\0';
        int nwrite = 0;
        n = 0;
        if (http_codes == BAD_REQUEST ||http_codes == NOT_IMPLEMENTED ||
                http_codes ==  NOT_FOUND ||
                (http_codes == OK && hh->method == "HEAD"))
        {
            while ((n = write(connfd, out_buf + nwrite, i)) != 0)
            {
                if (n == -1)
                {
                    if (errno == EINTR)
                        continue;
                    else
                    {
                        clear(connfd, hh);
                        return NULL;
                    }
                }
                nwrite += n;
            }
        } 
        if (http_codes == OK)
        {
            if (hh->method == "GET")
            {
                while ((n = write(connfd, out_buf + nwrite, i)) != 0)
                {
                    if (n == -1)
                    {
                        if (errno == EINTR)
                            continue;
                        else
                        {
                            clear(connfd, hh);
                            return NULL;
                        }
                    }
                    nwrite += n;
                }
                string real_url = get_real_url(hh->url);
                int fd = open(real_url.c_str(), O_RDONLY);
                // c_str()返回c风格字符串
                int file_size = get_file_length(real_url.c_str());
                int nwrite = 0;
                while (1)
                {
                    // 使用sendfile减少数据拷贝次数
                    if ((sendfile(connfd, fd, (off_t*)&nwrite, file_size)) < 0)
                        perror("sendfile") ;
                    if (nwrite < file_size)
                        continue;
                }
            }
            free(out_buf);
        }
    }
    clear(connfd, hh);
}

// 清除函数
void clear(int connfd, HttpHeader* hh)
{
    free_http_header(hh);
    close(connfd);
    thread_num_minus1();
}

// thread_num原子加１
inline void thread_num_add1()
{
    pthread_mutex_lock(&thread_num_mutex);
    ++thread_num;
    pthread_mutex_unlock(&thread_num_mutex);
}

// thread_num原子减１
inline void thread_num_minus1()
{
    pthread_mutex_lock(&thread_num_mutex);
    --thread_num;
    pthread_mutex_unlock(&thread_num_mutex);
}

// thread_num原子读
int32_t thread_num_read()
{
}

// HttpHeader处理函数，根据解析下来的HttpHeader来处理客户的请求
// result保存了处理结果，即http响应包
// 返回值：HTTP状态码
int do_http_header(HttpHeader* hh, string& result)
{
	char status_line[256] = {0};
	string crlf("\r\n");
	string server("Server: Marvin's http\r\n");
	string Public("Public: GET, HEAD\r\n");
	string content_base = "Content-Base: " + domain + crlf;
	string date = "Date: " + gmt_time() + crlf;

	string content_length("Content-Length: ");
	string content_location("Content-Location: ");
	string last_modified("Last-Modified: ");
	string body("");

	if(hh == NULL)
	{
		snprintf(status_line, sizeof(status_line), "HTTP/1.1 %d %s\r\n", 
			        BAD_REQUEST, get_state_by_codes(BAD_REQUEST));
		result = status_line + crlf;
		return BAD_REQUEST;
	}

	string method = hh->method;
	string real_url = get_real_url(hh->url);
	string version = hh->version;
	if(method == "GET" || method == "HEAD")
	{
		if(file_existed(real_url.c_str()) == false)
		{
			snprintf(status_line, sizeof(status_line), "HTTP/1.1 %d %s\r\n", 
				NOT_FOUND, get_state_by_codes(NOT_FOUND));
			result += (status_line + server + date + crlf); 
			return NOT_FOUND;
		}
		else
		{
			int len = get_file_length(real_url.c_str());
			snprintf(status_line, sizeof(status_line), "HTTP/1.1 %d %s\r\n", 
				        OK, get_state_by_codes(OK));
			result += status_line;
			snprintf(status_line, sizeof(status_line), "%d\r\n", len);
			result += content_length + status_line;
			result += server + content_base + date;
			result += last_modified + get_file_modified_time(real_url.c_str()) + crlf + crlf;
		}
	}
	else if(method == "PUT")
	{
		snprintf(status_line, sizeof(status_line), "HTTP/1.1 %d %s\r\n", 
				    NOT_IMPLEMENTED, get_state_by_codes(NOT_IMPLEMENTED));
		result += status_line + server + Public + date + crlf;
		return NOT_IMPLEMENTED;
	}
	else if(method == "DELETE")
	{
		snprintf(status_line, sizeof(status_line), "HTTP/1.1 %d %s\r\n", 
				    NOT_IMPLEMENTED, get_state_by_codes(NOT_IMPLEMENTED));
		result += status_line + server + Public + date + crlf;
		return NOT_IMPLEMENTED;
	}
	else if(method == "POST")
	{
		snprintf(status_line, sizeof(status_line), "HTTP/1.1 %d %s\r\n", 
				    NOT_IMPLEMENTED, get_state_by_codes(NOT_IMPLEMENTED));
		result += status_line + server + Public + date + crlf;
		return NOT_IMPLEMENTED;
	}
	else
	{
		snprintf(status_line, sizeof(status_line), "HTTP/1.1 %d %s\r\n", 
			        BAD_REQUEST, get_state_by_codes(BAD_REQUEST));
		result = status_line + crlf;
		return BAD_REQUEST;
	}

	return OK;
}

// 通过HTTP状态码返回友好语句
const char* get_state_by_codes(int http_codes)
{
	switch (http_codes)
	{
		case OK:
			return "ok";
		case BAD_REQUEST:
			return "badrequest";
		case FORBIDDEN:
			return "forbidden";
		case NOT_FOUND:
			return "notfound";
		case NOT_IMPLEMENTED:
			return "notimplemented";
		default:
			break;
	}

	return NULL;
}