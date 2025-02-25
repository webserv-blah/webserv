#!/bin/bash

BASIC="\
GET /index.html HTTP/1.1\r\n\
Host: localhost\r\n\
Connection: keep-alive\r\n\
Content-Length: 4\r\n\
\r\n\
hi\r\n\
"

CHUNK="\
POST /upload HTTP/1.1\r\n\
Host: example.com\r\n\
Transfer-Encoding: chunked\r\n\
Content-Type: text/plain\r\n\
\r\n\
4\r\n\
Wiki\r\n\
6\r\n\
pedia\n\r\n\
10\r\n\
 in\n12345\n\r\n\
0\r\n\
"

ERROR="\
GET /index.html HTTP/1.1\r\n\
Host: localhost\r\n\
Connection: keep-alive\r\n\
\r\n\
hi\r\n\
"
#여기 에러케이스 더 작성해봐야함...

export HTTP_REQUEST=$(printf "%b" "$ERROR")
