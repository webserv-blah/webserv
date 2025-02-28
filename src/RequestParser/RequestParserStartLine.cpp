#include "RequestParser.hpp"
#include "../include/errorUtils.hpp"
#include <sstream>

// 2. Start-line 한 줄을 파싱하는 함수
// line: \r\n기준으로 나뉜 요청 데이터의 첫 줄
// reqMsg: 현재 요청 데이터를 파싱하고 저장할 RequestMessage
EnumStatusCode RequestParser::parseStartLine(const std::string &line, RequestMessage &reqMsg) {
	std::istringstream iss(line);
	std::string buffer;

	if (std::getline(iss, buffer, ' ')) {
		if (buffer == "GET")
			reqMsg.setMethod(GET);
		else if (buffer == "POST")
			reqMsg.setMethod(POST);
		else if (buffer == "DELETE")
			reqMsg.setMethod(DELETE);
		else {
			reqMsg.setStatus(REQ_ERROR);
			return NOT_IMPLEMENTED;//status code: 구현되지 않은 메서드
		}
	} else {
		webserv::logError(ERROR, "BAD_REQUEST", 
			"method format error", 
			"RequestParser::parseStartLine");
		return BAD_REQUEST;
	}

	if (std::getline(iss, buffer, ' ')) {
		if (buffer.size() > this->uriMaxLength_)
			return URI_TOO_LONG;//status code: URI가 서버지원사이즈보다 큼
		reqMsg.setTargetURI(buffer);
	} else {
		webserv::logError(ERROR, "BAD_REQUEST", 
			"targeturi format error", 
			"RequestParser::parseStartLine");
		return BAD_REQUEST;//status code: start-line의 "method URI version" 형식이 유효하지 않음
	}

	std::getline(iss, buffer, '\0');
	if (buffer == "HTTP/1.1")
		reqMsg.setStatus(REQ_STARTLINE);
	else {
		reqMsg.setStatus(REQ_ERROR);
		webserv::logError(ERROR, "BAD_REQUEST", 
			"http version error", 
			"RequestParser::parseStartLine");
		return BAD_REQUEST;//status code: HTTP 버전 오류
	}
	return NONE_STATUS_CODE;
}
