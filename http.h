#ifndef MYHTTP_HTTP_H_
#define MYHTTP_HTTP_H_

#include "utility.h"
#include "parse.h"

#include <sys/socket.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <sys/epoll.h>
#include <strings.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <pthread.h>

#include <string>
#include <fstream>
#include <sstream>
#include <map>
#include <iostream>

using std::string;

struct EpollfdConnfd
{
    int epollfd;
    int connfd;
}; 

//**********常量定义**********
#define MAX_EVENTS  1024 // epoll最大监听事件数
#define MAX_BACKLOG 100 // 监听队列最大数

//**********配置文件相关值**********
string domain("");
string docroot("");

//**********MIME定义**********
struct MimeNode
{
    const char* type;
    const char* value;
};

MimeNode mime[] =
{
	{".html", "text/html"},
	{".xml", "text/xml"},
	{".xhtml", "application/xhtml+xml"},
	{".txt", "text/plain"},
	{".rtf", "application/rtf"},
	{".pdf", "application/pdf"},
	{".word", "application/msword"},
	{".png", "image/png"},
	{".gif", "image/gif"},
	{".jpg", "image/jpeg"},
	{".jpeg", "image/jpeg"},
	{".au", "audio/basic"},
	{".mpeg", "video/mpeg"},
	{".mpg", "video/mpeg"},
	{".avi", "video/x-msvideo"},
	{".gz", "application/x-gzip"},
	{".tar", "application/x-tar"},
	{NULL ,NULL}
};

// 将MIME的type转换为相应的value
// 返回NULL表示type在MIME中不存在
const char* mime_type2value(const char* type)
{
    for (int i = 0; mime[i].type != NULL; ++i)
    {
        if (strcmp(type, mime[i].type) == 0)
            return mime[i].value;
    }
    return NULL;
}

//**********HTTP响应码**********
#define CONTINUE                100 // 客户端继续请求
#define SWITCH                  101 // 转换协议
#define OK                      200 // 服务器已成功处理请求 
#define CREATED                 201 // 已创建
#define ACCEPTED                202 // 服务器接受但未处理
#define MOVED                   301 // 已移走
#define FOUND                   302 //　临时移走 
#define SEE_OTHER               303 // 参见其他信息
#define NOT_MODIFIED            304 // 未修改
#define PROXY                   305 // 使用代理
#define BAD_REQUEST             400 // 错误请求
#define FORBIDDEN               403 // 拒绝请求 
#define NOT_FOUND               404 // 未找到
#define REQUEST_TIMEOUT         408 // 请求超时
#define SERVER_ERROR            500 // 服务器内部错误
#define NOT_IMPLEMENTED         501 // 服务器未实现
#define BAD_GATEWAY             502 // 错误的网关
#define UNAVAILABLE             503 // 服务无法获得
#define VERSION_NOT_SUPPORTED   505 // 不支持的HTTP版本
//others...

// 通过HTTP状态码返回相应语句
inline const char* get_state_by_codes(int http_codes);

//**********HTTP响应首部**********
#define ACCEPTRANGE_HEAD		"Accpet-Range"
#define	AGE_HEAD 				"Age"
#define	ALLOW_HEAD				"Allow"
#define	CONTENTBASE_HEAD		"Content-Base"
#define	CONTENTLENGTH_HEAD		"Content-Length"
#define	CONTENTLOCATION_HEAD	"Content-Location"
#define	CONTENTRANGE_HEAD		"Content-Range"
#define	CONTENTTYPE_HEAD		"Content-Type"
#define	DATE_HEAD				"Date"
#define	EXPIRES_HEAD			"Expires"
#define	LAST_MODIFIED_HEAD		"Last-Modified"
#define	LOCATION_HEAD 			"Location"
#define	PUBLIC_HEAD				"Public"
#define RANGE_HEAD 				"Range"
#define	SERVER_HEAD				"Server"

#endif