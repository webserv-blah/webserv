#ifndef CGI_PROCESS_INFO_HPP
#define CGI_PROCESS_INFO_HPP

#include <unistd.h>  // close() 및 pid_t 정의
#include <sstream>  // std::ostringstream 사용을 위한 헤더 포함

class CgiProcessInfo {
public:
	pid_t pid_;
	int outPipe_;
	std::ostringstream	cgiResultBuffer;
};

#endif
