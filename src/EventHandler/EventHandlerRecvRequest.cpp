#define _DARWIN_C_SOURCE
#include "EventHandler.hpp"
#include <iostream>

#ifndef BUFFER_SIZE
# define BUFFER_SIZE 8192 //== 8KB
#endif

EnumSesStatus EventHandler::recvRequest(ClientSession &curSession) {
	std::string buffer;
	buffer.resize(BUFFER_SIZE);

	ssize_t res = recv(curSession.getClientFd(), &buffer[0], BUFFER_SIZE, MSG_DONTWAIT);
	if (res == -1) {
		std::cerr << "Error: systemcall error" << std::endl;
		return (CONNECTION_ERROR);
	} else if (res > 0) {
		EnumSesStatus requestResult;
		requestResult = curSession.implementReqMsg(this->parser_, buffer.substr(res));
		return requestResult;
	}
}
