#include "parse.h"

#include <cctype> // for isspace isalpha
#include <sstream>

http_header* alloc_http_header()
{
    http_header* p_http_header = (http_header*)new http_header;
    if (p_http_header == NULL) 
    {
        perror("http_header alloc failed");
    }
    return p_http_header;
}

void free_http_header(http_header* hh)
{
    if (hh == NULL)
        return;
        delete hh;
}

bool parse_http_request(const string& http_request, http_header* result)
{
    if (http_request.empty())
    {
        perror("http_request is empty!");
        return false;
    }
    if (result == NULL)
    {
        perror("result is NULL, please malloc first!");
        return false;
    }

    // http规定的换行方式是CRLF，即\r\n
    // windows下的换行标志是CRLF, UXIX下是\n，因此为了兼容不同平台，
    // 都以\n作为换行标志来处理
    string lf("\n"); 
    size_t prev = 0, next = 0;

    // 此处开始解析http_request
    if ((next = http_request.find(lf, prev)) != string::npos)
    {
        string first_line(http_request.substr(prev, next - prev));
        prev = next + 1; // 下一行起始位置
        if (string(http_request, next -1, 1) == "\r")
            --next;
        std::stringstream string_stream(first_line);
        string_stream >> (result->method);
        string_stream >> (result->url);
        string_stream >> (result->version);
    }
    else
    {
        perror("parse_http_request error: http_request has"
               " not \\n");
        return false;
    }

    string lf2("\n\n");
    string crlf2("\r\n\r\n");
    size_t pos_lf2 = http_request.find(lf2, prev);
    size_t pos_crlf2 = http_request.find(crlf2, prev);
    if (pos_lf2 == string::npos && pos_crlf2 == string::npos)
    {
        perror("parse_http_request error: http_request has" 
                " not \\r\\n\\r\\n or \\n\\n");
        return false;
    }

    // 解析header
    size_t pos = ((pos_lf2 == string::npos) ? pos_crlf2 : pos_lf2);
    string buff, key, value;
    while(1)
    {
        next = http_request.find(lf, prev);
        // 如果找到的next不超过lf2或者crlf2
        if (next <= pos)
        {
            buff = http_request.substr(prev, next - prev);
            size_t end = 0;
            // 跳过前置空白字符，达到首部关键字的起始位置
            for (; std::isspace(buff[end]); ++end)
                ;
            int beg = end;
            //　到达首部关键字的结束位置
            for (; buff[end] != ':' && !std::isspace(buff[end]); ++end)
                ;
            key = buff.substr(beg, end - beg);

            for (; !std::isalnum(buff[end]); ++end)
                ;
            beg = end;
            for (; next != end; ++end)
                ;
            value = buff.substr(beg, end - beg);
            result->header.insert(std::make_pair(key, value));

            prev = next + 1;
        }
        else
        {
            break;
        }
    }

    // 解析请求包的实体（一般不存在）
    size_t tmp = (pos_lf2 == string::npos ? 4 : 2);
    result->body = http_request.substr(pos + tmp, http_request.size() - pos - tmp);

    return true;
}