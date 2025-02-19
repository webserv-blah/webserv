#!/bin/bash

export HTTP_REQUEST=$(echo -e $ERROR)

BASIC="\
GET /index.html HTTP/1.1\r\n\
Host: localhost\r\n\
Connection: keep-alive\r\n\
Content-Length: 4\r\n\
\r\n\
hi\r\n\
"

CHUNK="\
GET /index.html HTTP/1.1\r\n\
Host: localhost\r\n\
Connection: keep-alive\r\n\
\r\n\
4\r\n\
hi\r\n\
7\r\n\
hello\r\n\
0\r\n\
"

ERROR="\
GET /index.html HTTP/1.1\r\n\
\r\n\
Host: localhost\r\n\
Connection: keep-alive\r\n\
Content-Length: 4\r\n\
\r\n\
hi\r\n\
"