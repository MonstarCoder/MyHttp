#ifndef MYHTTP_PARSE_H_
#define MYHTTP_PARSE_H_

#include <stdlib.h>
#include <stdio.h>

#include <string>
#include <map>
#include <utility>

using std::string;

// 解析信息
typedef struct HttpHeader
{
    string method; // 请求方式
    string url; // 统一资源定位符
    string version; // http版本
    std::map<string, string> header; // 一条头部信息
    string body; // 主体部分
} http_header;

//分配内存给http_header,返回NULL表示分配失败
http_header* alloc_http_header();

//回收分配给http_header的内存
void free_http_header(http_header* hh);

// 解析http request, result保存http_header
// 返回值：true表示解析成功，否则false失败
bool parse_http_request(const string& http_request, http_header* result);

#endif
