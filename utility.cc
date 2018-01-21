#include "utility.h"

// 解析文件的键值对，如：key = docroot, value = /home/...
std::map<string, int> config_keyword_map;

// 配置文件信息
// #define DOMAIN 2
// #define DOCROOT 1
// extern string domain;
// extern string docroot;

// 返回GMT时间，如：Sat, 20 Jan 2018 07:29:15 GMT
// 参见APUE 6.10
string gmt_time()
{
	time_t now;
	struct tm *time_now;
	string str_time;

	time(&now);
	time_now = gmtime(&now);

	switch(time_now->tm_wday)
	{
		case 0:
			str_time += "Sun, ";
			break;
		case 1:
			str_time += "Mon, ";
			break;
		case 2:
			str_time += "Tue, ";
			break;
		case 3:
			str_time += "Wed, ";
			break;
		case 4:
			str_time += "Thu, ";
			break;
		case 5:
			str_time += "Fri, ";
			break;
		case 6:
			str_time += "Sat, ";
			break;
	}
	char buf[16];
	snprintf(buf, sizeof(buf), "%d ", time_now->tm_mday);
	str_time += string(buf);
	switch(time_now->tm_mon)
	{
		case 0:
			str_time += "Jan ";
			break;
		case 1:
			str_time += "Feb ";
			break;
		case 2:
			str_time += "Mar ";
			break;
		case 3:
			str_time += "Apr ";
			break;
		case 4:
			str_time += "May ";
			break;
		case 5:
			str_time += "Jun ";
			break;
		case 6:
			str_time += "Jul ";
			break;
		case 7:
			str_time += "Aug ";
			break;
		case 8:
			str_time += "Sep ";
			break;
		case 9:
			str_time += "Oct ";
			break;
		case 10:
			str_time += "Nov ";
			break;
		case 11:
			str_time += "Dec ";
			break;
	}
	snprintf(buf, sizeof(buf), "%d", time_now->tm_year + 1900);
	str_time += string(buf);
	snprintf(buf, sizeof(buf), " %d:%d:%d ", time_now->tm_hour, time_now->tm_min, time_now->tm_sec);
	str_time += string(buf);

	str_time += "GMT";

	return str_time;
}

// 返回真正的url
string get_real_url(const string& url)
{
    string real_url, url2;
    int n = 0;
    // 去除域名
    if ((n = url.find(domain, 0)) != string::npos)
        url2 = url.substr(domain.size(), url.size() - domain.size());
    else
        url2 = url;

    // 配置项docroot末尾有'/'
    if (docroot[docroot.size() - 1] == '/')
    {
        if (url2[0] == '/')
            real_url = docroot + url2.erase(0, 1);
        else
            real_url = docroot + url2;
    }
    else
    {
        if (url2[0] == '/')
            real_url = docroot + url2;
        else
            real_url = docroot + '/' + url2;
    }

    return real_url;
}

// 返回文件长度(绝对路径＋文件名)
int get_file_length(const char* path)
{
    struct stat buf;
    int ret = stat(path, &buf);
    if (ret == -1)
    {
        perror("get_file_length error");
        exit(-1);
    }
    return (int)buf.st_size;
}

// 获取文件最后修改时间
string get_file_modified_time(const char* path)
{
    struct stat buf;
    int ret = stat(path, &buf);
    if (ret == -1)
    {
        perror("get_file_modified_time error");
        exit(-1);
    }
    char array[32] = {0};
    snprintf(array, sizeof(array), "%s", ctime(&buf.st_mtime));
    return string(array, array + strlen(array));
}

// 初始化全局变量config_keyword_map
void init_config_keyword_map()
{
    config_keyword_map.insert(std::make_pair("docroot", DOCROOT));
    config_keyword_map.insert(std::make_pair("domain", DOMAIN));
}

// 解析配置文件
bool parse_config(const char* path)
{
    init_config_keyword_map();
 	int ret = 0;
 	std::fstream infile(path, std::fstream::in);
 	string line, word;
 	if(!infile)
 	{
 		printf("can't open%s\n", path);
 		infile.close();
 		return false;
 	}
 	while(std::getline(infile, line))
 	{
 		std::stringstream stream(line);
 		stream >> word; // keyword
 		std::map<string, int>::const_iterator cit= config_keyword_map.find(word);
 		if(cit == config_keyword_map.end())
 		{
 			printf("can't find keyword\n");
 			infile.close();
 			return false;
 		}
 		switch (cit->second)
 		{
 			case DOCROOT:
 				stream >> docroot;
 				break;
 			case DOMAIN:
 				stream >> domain;
 				break;
 			default :
 				infile.close();
 				return false;
 		}
 	}
 	infile.close();
 	return true;
}

// 设置文件描述符为非阻塞模式
void set_nonblock(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0)
    {
        perror("fcntl: F_GETFL");
        exit(-1);
    }
    flags |= O_NONBLOCK;
    int ret = fcntl(fd, F_SETFL, flags);
    if (ret < 0)
    {
        perror("fcntl: F_SETFL");
        exit(-1);
    }
}

// 设置套接字选项SO_REUSEADDR
void set_reuse_addr(int sockfd)
{
    int on = 1;
    int ret = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    if (ret == -1)
    {
        perror("setsockopt: SO_REUSEADDR");
        exit(-1);
    }
}

// 关闭nagle
void set_off_tcp_nagle(int sockfd)
{
    int on = 1;
    int ret = setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &on, sizeof(on));
    if (ret == -1)
    {
        perror("setsockopt: TCP_NODELAY ON");
        exit(-1);
    }
}

// 开启nagle
void set_on_tcp_nagle(int sockfd)
{
 	int off = 0;
 	int ret = setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &off, sizeof(off));
 	if (ret == -1)
 	{
 		perror("setsockopt: TCP_NODELAY OFF");
		exit(-1);
 	}
}

// 开启套接字TCP_CORK选项
void set_on_tcp_cork(int sockfd)
{
 	int on = 1;
 	int ret = setsockopt(sockfd, SOL_TCP, TCP_CORK, &on, sizeof(on));
 	if (ret == -1)
 	{
 		perror("setsockopt: TCP_CORK ON");
		exit(-1);
 	}
}

// 关闭套接字TCP_CORK选项
void set_off_tcp_cork(int sockfd)
{
 	int off = 0;
 	int ret = setsockopt(sockfd, SOL_TCP, TCP_CORK, &off, sizeof(off));
 	if (ret == -1)
 	{
 		perror("setsockopt: TCP_CORK OFF");
		exit(-1);
 	}
}

// 设置套接字SO_RCVTIMEO选项，接收超时
// 函数参数：sec秒，msec微妙
void set_recv_timeo(int sockfd, int sec, int msec)
{
 	struct timeval time= {sec, msec};
 	int ret = setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &time, sizeof(time));
 	if (ret == -1)
 	{
 		perror("setsockopt: SO_RCVTIMEO");
		exit(-1);
 	}
}

// 设置套接字SO_SNDTIMEO选项，发送超时
void set_send_timeo(int sockfd, int sec, int msec)
{
 	struct timeval time= {sec, msec};
 	int ret = setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &time, sizeof(time));
 	if (ret == -1)
 	{
 		perror("setsockopt: SO_SNDTIMEO");
		exit(-1);
 	}
}

//**********系统函数的包装**********
int my_socket(int domain, int type, int protocol)
{
	int listen_fd;
	if ((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("socket");
		exit(-1);
	}
	return listen_fd;
}

void my_bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
	if (bind(sockfd, addr, addrlen) == -1)
	{
		perror("bind");
		exit(-1);
	}
}

void my_listen(int sockfd, int backlog)
{
	if (listen(sockfd, backlog) == -1)
	{
		perror("listen");
		exit(-1);
	}
}

int my_accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
	int ret_fd = 0;
    while(1)
	{	
		ret_fd = accept(sockfd, addr, addrlen);
		if (ret_fd > 0)
			break;
		else if (ret_fd == -1)
		{
			//由于我们把监听套接字设置为了非阻塞模式
			if (errno != EAGAIN && errno != EPROTO && errno != EWOULDBLOCK
			 		&& errno != EINTR && errno != ECONNABORTED)
			{	
				perror("accept");
				exit(-1);
			}
		}
		else
			continue;
	}
	return ret_fd;
}

struct servent* my_getservbyname(const char *name, const char *proto)
{
	struct servent 	*pservent;
	if ((pservent = getservbyname(name, proto)) == NULL)
	{
		perror("getservbyname");
		exit(-1);
	}
	return pservent;
}

int my_epoll_create(int size)
{
	int epollfd;
	epollfd = epoll_create(size);
	if (epollfd == -1)
	{
		perror("epoll_create");
		exit(-1);
	}
	return epollfd;
}

void my_epoll_ctl(int epfd, int op, int fd, struct epoll_event *event)
{
	if (epoll_ctl(epfd, op, fd, event) == -1)
	{
		perror("epoll_ctl");
		exit(-1);
	}	
}

int my_epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout)
{
    while(1)
    {
        int nfds = epoll_wait(epfd, events, maxevents, timeout);
        if (nfds == -1) //&& errno != EINTR)
        {
            if (errno != EINTR)
            {
                perror("epoll_wait");
                exit(-1);
            }
            else
                continue;
        }
	    return nfds;
    }
}

void *my_calloc(size_t nmemb, size_t size)
{
	void *ptr = calloc(nmemb, size);
	if (ptr == NULL)
	{
		perror("my_calloc");
		exit(-1);
	}
	return ptr;
}

void *my_malloc(size_t size)
{
	void *ptr = malloc(size);
	if(NULL == ptr)
	{
		perror("my_malloc");
		exit(-1);
	}
	return ptr;
}

void my_free(void *ptr)
{
	free(ptr);
}