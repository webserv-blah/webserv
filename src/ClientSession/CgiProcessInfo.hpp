#ifndef CGI_PROCESS_INFO_HPP
#define CGI_PROCESS_INFO_HPP

#include <unistd.h>  // close() 및 pid_t 정의
#include <sstream>  // std::ostringstream 사용을 위한 헤더 포함
#include <sys/wait.h>  // waitpid() 사용을 위한 헤더 포함
#include <signal.h>  // kill() 사용을 위한 헤더 포함
#include "errorUtils.hpp"  // 에러 로깅을 위한 헤더 포함

class CgiProcessInfo {
public:
	CgiProcessInfo() : pid_(-1), outPipe_(-1), isProcessing_(false) {}

	pid_t pid_;
	int outPipe_;
	std::ostringstream	cgiResultBuffer_;
	bool	isProcessing_;

	// 자식 프로세스를 종료하고 pip를 닫음
	void cleanup() {
		close(outPipe_);
		//WNOHANG으로 wait
		int status;
		waitpid(pid_, &status, WNOHANG);
		if (!WIFEXITED(status)) {
            //자식 프로세스 종료
			kill(pid_, SIGKILL);
		}
		pid_ = -1;
		outPipe_ = -1;
		cgiResultBuffer_.str("");
		isProcessing_ = false;
	}
};

#endif
