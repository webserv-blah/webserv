#include "EventHandler.hpp"
#include <iostream>
#define _DARWIN_C_SOURCE
#include <sys/socket.h>
#include "../include/errorUtils.hpp"

#ifndef BUFFER_SIZE
# define BUFFER_SIZE 8192 //== 8KB
#endif

EnumSesStatus EventHandler::recvRequest(ClientSession &curSession) {
	std::string buffer;
	buffer.resize(BUFFER_SIZE);

	ssize_t res = recv(curSession.getClientFd(), &buffer[0], BUFFER_SIZE, MSG_DONTWAIT);
	if (res == -1) {	
		// 심각한 오류 발생 시 ERROR 레벨로 로그만 출력하고 연결 종료 (서버는 계속 실행)
		webserv::logSystemError(ERROR, "recv", 
		                      "Client fd: " + utils::size_t_tos(curSession.getClientFd()),
		                      "EventHandler::recvRequest");
		return CONNECTION_CLOSED;	//errno)그 외, 통신을 할 수 없는 상태이거나, 메모리 문제로 치명적인 경우
	} else if (res == 0) {			//클라이언트 측에서 연결을 종료한 경우
		return CONNECTION_CLOSED;
	} else {
		// DEBUG_LOG("[EventHandler]Received MSG:\n" << buffer.substr(0, res))

		// 해당 ClientSession의 RequestMessage를 파싱하기 전에 Body의 최대 길이를 설정
		const RequestConfig *config = curSession.getConfig();
		const size_t bodyMax = (config == NULL) ? BODY_MAX_LENGTH : config->clientMaxBodySize_.value();
		this->parser_.setConfigBodyLength(bodyMax);

		// 이전에 버퍼 저장해놓은 데이터와 새로운 요청 데이터를 가지고 파싱
		EnumStatusCode statusCode = this->parser_.parse(buffer.substr(0, res), curSession);
		curSession.setErrorStatusCode(statusCode);

		// 파싱이 끝나고 나서, 에러 status code와 RequestMessage의 상태를 점검함
		EnumSesStatus result;
		if (curSession.getErrorStatusCode() != NONE_STATUS_CODE)
			result = REQUEST_ERROR;
		else if (curSession.getReqMsg()->getStatus() == REQ_DONE)
			result = READ_COMPLETE;
		else
			result = READ_CONTINUE;

		// 파싱이 완전히 종료되는 경우에 Config 누락 방지
		if ((result == READ_COMPLETE || result == REQUEST_ERROR)
		&& curSession.getConfig() == NULL) {
			const GlobalConfig &globalConfig = GlobalConfig::getInstance();
			const RequestMessage *curMsg = curSession.getReqMsg();
			size_t colonPos = curMsg->getMetaHost().find(":", 0);
			std::string onlyDomainName = (colonPos == std::string::npos) ? curMsg->getMetaHost() : curMsg->getMetaHost().substr(0, colonPos);
			curSession.setConfig(globalConfig.findRequestConfig(curSession.getListenFd(), onlyDomainName, curMsg->getTargetURI()));
		}
		return result;
	}
}