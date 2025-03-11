#ifndef CGI_PROCESS_INFO_HPP
#define CGI_PROCESS_INFO_HPP

#include <unistd.h>  // close() 및 pid_t 정의
#include <string>
#include <sstream>

class CgiProcessInfo {
public:
	pid_t pid_;
	int inPipe_[2];
	int outPipe_[2];
	bool finished_;
};

#endif
