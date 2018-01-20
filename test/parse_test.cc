#include "../parse.h"

#include <iostream>
using std::cout;
using std::endl; 

int main()
{
    http_header *hh = alloc_http_header();
	string http_request(
        "GET /home/marvin/test HTTP1.1\r\n\
Lengh: 8080\r\n\
Date: Fri Mon 2018\r\n\
\r\n\
<html>\n\
sb\n\
</html>"
    );
    // cout << http_request << endl;
	parse_http_request(http_request, hh);
	cout << hh->method << " " << hh->url << " " << 
        hh->version << endl;
	free_http_header(hh);
	return 0;
}