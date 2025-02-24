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
		if (errno == EAGAIN || errno == EWOULDBLOCK)//non-blocking 모드에서 읽을 데이터가 없어 즉시 반환된 경우
			return READ_DEFFERED;//다음 이벤트를 기다려야함
		if (errno == EINTR)//인터럽트, 시스템 호출 중 시그널에 의해 중단되어 호출이 완료되지 않은 경우
			return READ_CONTINUE;//일반적으로 재시도
		//통신을 할 수 없는 상태이거나, 메모리 문제로 치명적인 경우
		return CONNECTION_CLOSED;
	} else if (res == 0) {//클라이언트 측에서 연결을 종료한 경우
		return CONNECTION_CLOSED;
	} else {
		EnumSesStatus requestResult;
		requestResult = curSession.implementReqMsg(this->parser_, buffer.substr(res));
		return requestResult;
	}
}
