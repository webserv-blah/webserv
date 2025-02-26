#include "EventHandler.hpp"
#include <iostream>
#define _DARWIN_C_SOURCE
#include <sys/socket.h>

#ifndef BUFFER_SIZE
# define BUFFER_SIZE 8192 //== 8KB
#endif

EnumSesStatus EventHandler::recvRequest(ClientSession &curSession) {
	std::string buffer;
	buffer.resize(BUFFER_SIZE);

	ssize_t res = recv(curSession.getClientFd(), &buffer[0], BUFFER_SIZE, MSG_DONTWAIT);
	if (res == -1) {
		if (errno == EAGAIN
		||  errno == EWOULDBLOCK	//errno)non-blocking 모드에서 읽을 데이터가 없어 즉시 반환된 경우
		||  errno == EINTR)			//errno)인터럽트, 시스템 호출 중 시그널에 의해 중단되어 호출이 완료되지 않은 경우
			return READ_CONTINUE;
		return CONNECTION_CLOSED;	//errno)그 외, 통신을 할 수 없는 상태이거나, 메모리 문제로 치명적인 경우
	} else if (res == 0) {			//클라이언트 측에서 연결을 종료한 경우
		return CONNECTION_CLOSED;
	} else {
		// 해당 ClientSession의 RequestMessage를 파싱하기 전에 Body의 최대 길이를 설정
		const RequestConfig *config = curSession.getConfigPtr();
		const size_t bodyMax = (config == NULL) ? BODY_MAX_LENGTH : config->clientMaxBodySize_.value();
		this->parser_.setConfigBodyLength(bodyMax);

		// 이전에 버퍼 저장해놓은 데이터와 새로운 요청 데이터를 가지고 파싱
		EnumStatusCode statusCode = this->parser_.parse(buffer.substr(res), curSession);
		curSession.setErrorStatusCode(statusCode);

		// 파싱이 끝나고 나서, 에러 status code와 RequestMessage의 상태를 점검함
		if (curSession.getErrorStatusCode() != NONE_STATUS_CODE)
			return REQUEST_ERROR;
		else if (curSession.getReqMsgPtr()->getStatus() == REQ_DONE)
			return READ_COMPLETE;
		return READ_CONTINUE;
	}
}
