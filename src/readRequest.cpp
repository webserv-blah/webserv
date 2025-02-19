#include <sys/socket.h>
#include <iostream>
#include <string>
#include "ClientSession.hpp"

#define BUFFER_SIZE 8192 //== 8KB

EnumSesStatus readRequest(ClientSession &curSession, RequestParser &parser) {
	/* RECV 함수
	std::string buffer;
	buffer.resize(BUFFER_SIZE);

	ssize_t res = recv(curSession.getClientFd(), &buffer[0], BUFFER_SIZE, MSG_DONTWAIT);
	if (res == -1) {
		std::cerr << "Error: systemcall error" << std::endl;
		return (CONNECTION_ERROR);
	} else if (res > 0) {
		EnumSesStatus requestResult;
		requestResult = curSession.implementReqMsg(parser, std::string(static_cast<char *>(buffer), res));
		if (requestResult == READ_COMPLETE)
			//handleAndResponse();
		return requestResult;
	}
	*/
	
	const char* data = std::getenv("HTTP_REQUEST");
	if (data == NULL) {
		std::cerr << "환경변수 HTTP_REQUEST가 설정되지 않았습니다.\n";
		exit(1);
	}
	EnumSesStatus requestResult = curSession.implementReqMsg(parser, std::string(data));
	//if (requestResult == READ_COMPLETE)
		//handleAndResponse();
	return requestResult;
}
