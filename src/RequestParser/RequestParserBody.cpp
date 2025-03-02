#include "RequestParser.hpp"
#include "../utils/utils.hpp"
#include "../include/errorUtils.hpp"
#include <sstream>

// 4. Body를 파싱하는 함수, Content-Length의 길이에 따라 예외 처리됨. 유효한 문자열은 기존의 Body의 추가함
// readBuffer: Body로 파싱할 요청 데이터이자, 남은 데이터를 저장할 ClientSession의 readBuffer
// reqMsg: 현재 요청 데이터를 파싱하고 저장할 RequestMessage
EnumStatusCode RequestParser::parseBody(std::string &readBuffer, RequestMessage &reqMsg) {
	const size_t contentLength = reqMsg.getMetaContentLength();

	// 4-1. 최종 body의 길이 >= 현재 저장된 body길이 + 파싱하려는 길이
	if (contentLength >= reqMsg.getBodyLength() + readBuffer.length()) {
		if (this->bodyMaxLength_ < reqMsg.getBodyLength() + readBuffer.length()) {
			webserv::logError(ERROR, "CONTENT_TOO_LARGE", 
				"Body size is larger than the Config", 
				"RequestParser::parseBody");
			return CONTENT_TOO_LARGE;//status code: Body가 설정보다 큼 “Request Entity Too Large”
		}

		reqMsg.addBody(readBuffer);
		readBuffer = "";
		if (contentLength == reqMsg.getBodyLength()) {
			reqMsg.setStatus(REQ_DONE);
			return NONE_STATUS_CODE;
		} else {
			reqMsg.setStatus(REQ_BODY);
			return NONE_STATUS_CODE;
		}
	}

	// 4-2. 최종 body의 길이 < 현재 저장된 body길이 + 파싱하려는 길이
	// 우선, content length를 저장하되
	size_t substrSize = contentLength - reqMsg.getBodyLength();
	reqMsg.addBody(readBuffer.substr(0, substrSize));
	std::string remainData = readBuffer.substr(substrSize);
	
	// 일차적으로 완성된 request이지만, 두 가지 우려사항을 검증해야함
	const std::string methods[3] = {"GET", "POST", "DELETE"};
	bool suspicious = true;
	// 1) 남은 데이터가 유효한 메서드로 시작하면 -> 정상 요청 가능성
	for (size_t i = 0; i < 3; ++i) {
		if (remainData[0] == methods[i][0]) {
			suspicious = false;
			break;
		}
	}
	// 2) body 안에 메서드가 들어있으면 → 의심스러운 요청
	for (size_t i = 0; i < 3; ++i) {
		if (reqMsg.getBody().find(methods[i]) != std::string::npos) {
			suspicious = true;
			break;
		}
	}

	if (suspicious) {
		reqMsg.setStatus(REQ_ERROR);
		webserv::logError(ERROR, "BAD_REQUEST", 
			"reject suspicious Body", 
			"RequestParser::parseBody");
		return BAD_REQUEST;//status code: 의심스러운 Body 거부
	}
	reqMsg.setStatus(REQ_DONE);
	readBuffer = remainData;
	return NONE_STATUS_CODE;
}

// 5. 청크 전송 파싱하는 함수. Transfer-Encoding이 chunked여서 Body가 청크로 아루어진 경우
// readBuffer: Body로 파싱할 요청 데이터이자, 남은 데이터를 저장할 ClientSession의 readBuffer
// reqMsg: 현재 요청 데이터를 파싱하고 저장할 RequestMessage
EnumStatusCode RequestParser::cleanUpChunkedBody(std::string &readBuffer, RequestMessage &reqMsg) {
	size_t chunkSize;

	bool isStart = true;
	size_t cursorFront = 0;
	size_t cursorBack = 0;
	
	while (1) {
		size_t findResult = readBuffer.find(CRLF, cursorBack, 2);
		
		if (findResult != std::string::npos) {
			cursorFront = (isStart) ? 0 : cursorBack;
			cursorBack = findResult+2;
			isStart = false;
		} else {// \n이 나오지 않음
			if (readBuffer.find(LF, cursorBack) != std::string::npos) {
				webserv::logError(ERROR, "BAD_REQUEST",
					"single LF",
					"RequestParser::parse");
				return BAD_REQUEST;//status code: CRLF가 아닌, 단일 LF
			}
			readBuffer.erase(0, cursorFront);
			return NONE_STATUS_CODE;
		}
		
		chunkSize = utils::sto_size_t(readBuffer.substr(cursorFront, cursorBack-cursorFront-2));

		if (chunkSize + 2 > readBuffer.size() - cursorBack) {
			readBuffer.erase(0, cursorFront);
			return NONE_STATUS_CODE;
		}
		
		if (readBuffer.find(CRLF, cursorBack + chunkSize) != cursorBack) {
			webserv::logError(ERROR, "BAD_REQUEST", 
				"invalid chunk data format", 
				"RequestParser::cleanUpChunkedBody");
			return BAD_REQUEST;//status code: 유효하지 않은 청크 데이터 형식
		}
		
		if (chunkSize == 0) {
			reqMsg.setStatus(REQ_DONE);
			reqMsg.setMetaContentLength(reqMsg.getBodyLength());
			readBuffer.erase(0, cursorBack + chunkSize + 2);
			return NONE_STATUS_CODE;
		}

		if (this->bodyMaxLength_ < reqMsg.getBodyLength() + chunkSize) {
			webserv::logError(ERROR, "CONTENT_TOO_LARGE", 
				"Body size is larger than the Config", 
				"RequestParser::cleanUpChunkedBody");
			return CONTENT_TOO_LARGE;//status code: Body가 설정보다 큼 “Request Entity Too Large”
		}
		reqMsg.addBody(readBuffer.substr(cursorFront, chunkSize));
	}
	readBuffer.erase(0, cursorBack + chunkSize + 2);
	return NONE_STATUS_CODE;
}
