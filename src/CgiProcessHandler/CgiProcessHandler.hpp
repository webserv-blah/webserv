#ifndef CGI_PROCESS_INFO_HPP
#define CGI_PROCESS_INFO_HPP

#include <unistd.h>  // close() 및 pid_t 정의
#include <string>
#include <sstream>
#include "CgiProcessInfo.hpp"

class CgiProcessHandler {
public:
	std::vector<CgiProcessInfo*> cgiWaitList_;
    void    waitForCgi();
};

#endif
