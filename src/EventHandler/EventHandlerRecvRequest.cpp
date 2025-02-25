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
		// TO DO: RequestParser 내부로 이동
		//// 새로운 요청일 때, RequestMessage 동적할당
		//if (curSesson.getReqMsg() == NULL)
		//	curSesson.getReqMsg() = new RequestMessage();// 이후 요청처리(handler&builder) 완료 후, delete 필요

		EnumSesStatus requestResult;
		// 해당 ClientSession의 RequestMessage를 파싱하기 전에 Body의 최대 길이를 설정
		const RequestConfig *config = curSession.getConfig();
		const size_t BodyMax = (config == NULL) ? BODY_MAX_LENGTH : config->clientMaxBodySize_.value();

		// 이전에 버퍼 저장해놓은 데이터와 새로운 요청 데이터를 가지고 파싱
		EnumStatusCode statusCode = this->parser_.parse(buffer.substr(res), curSession);
		curSession.setErrorStatusCode(statusCode);

		// TO DO: RequestParser 내부로 이동
		//// field-line까지 다 읽은 후, 요청메시지에 맞는 RequestConfig를 설정하고 Host헤더필드 유무를 검증함
		//if (curSesson.getReqMsg()->getStatus() == REQ_HEADER_CRLF) {
		//	const GlobalConfig &globalConfig = GlobalConfig::getInstance();
		//	curSesson.getConfig() = globalConfig.findRequestConfig(curSesson.getListenFd(), curSesson.getReqMsg()->getMetaHost(), curSesson.getReqMsg()->getTargetURI());
		//	if (curSesson.getReqMsg()->getMetaHost().empty()) {
		//		curSesson.getErrorStatusCode() = BAD_REQUEST;
		//		return REQUEST_ERROR;
		//	}
		//	if (curSesson.getReqMsg()->getMetaContentLength() == 0
		//	&&  curSesson.getReqMsg()->getMetaTransferEncoding() == NONE_ENCODING) {
		//		curSesson.getReqMsg()->setStatus(REQ_DONE);
		//		return READ_COMPLETE;
		//	}
		//}

		// 파싱이 끝나고 나서, 에러 status code와 RequestMessage의 상태를 점검함
		if (curSession.getErrorStatusCode() != NONE_STATUS_CODE)
			return REQUEST_ERROR;
		else if (curSession.getReqMsg()->getStatus() == REQ_DONE)
			return READ_COMPLETE;
		return READ_CONTINUE;
	}
}
