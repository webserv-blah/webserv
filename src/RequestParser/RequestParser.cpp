#include "RequestParser.hpp"
#include "../GlobalConfig/GlobalConfig.hpp"
#include "../include/errorUtils.hpp"
#include <sstream>

//Body크기에 제한 없이 테스트하고 싶은 경우, BODY_MAX_LENGTH대신 std::numeric_limits을 사용할 수 있음
RequestParser::RequestParser() : uriMaxLength_(URI_MAX_LENGTH), bodyMaxLength_(BODY_MAX_LENGTH) {}
RequestParser::~RequestParser() {}

void RequestParser::setConfigBodyLength(size_t length) {
	this->bodyMaxLength_ = length;
}

// 1. 기본 파싱 로직 함수
// readData: recv함수로 읽은 요쳥 데이터
// curSession: 현재 클라이언트 소켓에 해당하는 데이터들(아래 내용)
//     reqMsg_: 현재 요청 데이터를 파싱하고 저장할 RequestMessage
//     readBuffer_: 기존 요청데이터를 파싱하고 남은 데이터
//     readCursor_: 버퍼의 마지막 위치를 기록
EnumStatusCode RequestParser::parse(const std::string &readData, ClientSession &curSession) {
	// 새로운 요청일 때, RequestMessage 동적할당
	if (curSession.getReqMsg() == NULL)
		curSession.setReqMsg(new RequestMessage());// 이후 요청처리(handler&builder) 완료 후, delete 필요 (ClientSession.resetRequest()메서드 사용)

	RequestMessage &reqMsg = curSession.accessReqMsg();
	std::string &readBuffer = curSession.accessReadBuffer();
	EnumReqStatus status = reqMsg.getStatus();

	bool isStart = true;
	size_t cursorFront = 0;
	size_t cursorBack = (isStart) ? 0 : readBuffer.size()-1;

	readBuffer.append(readData);

	// 1-1. Body가 아닌, start-line이나 field-line인 경우
	while (status != REQ_HEADER_CRLF && status != REQ_BODY) {
		size_t findResult = readBuffer.find(CRLF, cursorBack, 2);
		
		// find결과에 따라 cursor(인덱스 파싱)을 할지 다음 recv를 기다릴지 결정
		if (findResult != std::string::npos) {
			cursorFront = (isStart) ? 0 : cursorBack;
			cursorBack = findResult+2;
			isStart = false;
		} else {// \n이 나오지 않고 readData가 끝난 상태. 다음 loop로 넘어감
			if (readBuffer.find(LF, cursorBack+2) != std::string::npos) {
				webserv::logError(ERROR, "BAD_REQUEST",
					"single LF",
					"RequestParser::parse");
				return BAD_REQUEST;//status code: CRLF가 아닌, 단일 LF
			}
			readBuffer.erase(0, cursorBack);
			return NONE_STATUS_CODE;
		}
		
		// CRLF줄 처리
		if (cursorBack-cursorFront == 2) {
			status = this->handleCRLFLine(reqMsg.getStatus());
			reqMsg.setStatus(status);
			// 올바르지 않은 CRLF줄 에러
			if (status == REQ_ERROR) {
				webserv::logError(ERROR, "BAD_REQUEST", 
					"invalid CRLF location", 
					"RequestParser::parse");
				return BAD_REQUEST;//status code: 유효하지 않은 CRLF 위치
			}
			// field-line까지 다 읽은 후, 요청메시지에 맞는 RequestConfig를 설정하고 Host헤더필드 유무를 검증함
			if (status == REQ_HEADER_CRLF) {
				readBuffer.erase(0, cursorBack);

				// ClientSession에 Config설정
				const GlobalConfig &globalConfig = GlobalConfig::getInstance();
				curSession.setConfig(globalConfig.findRequestConfig(curSession.getListenFd(), reqMsg.getMetaHost(), reqMsg.getTargetURI()));

				// 헤더필드 검증 및 처리
				// 1) Host 헤더필드는 필수로 존재해야 함
				if (reqMsg.getMetaHost().empty()) {
					webserv::logError(ERROR, "BAD_REQUEST", 
						"Host does not exist", 
						"RequestParser::parse");
					return BAD_REQUEST;
				}
				// 2) targetURI와 Host 헤더필드 처리
				const std::string &targetURI = reqMsg.getTargetURI();
				const size_t schemaPos = targetURI.find("http://", 0);
				const size_t cursor = (schemaPos == std::string::npos) ? 0 : schemaPos;
				const size_t slashPos = targetURI.find("/", cursor);
				if (cursor != slashPos) {
					reqMsg.resetHostField(targetURI.substr(cursor, slashPos));
					reqMsg.setTargetURI(targetURI.substr(slashPos, targetURI.size()));
				}
				// 3) Body size가 정해진 것이 없을때, 종료 처리
				if (reqMsg.getMetaContentLength() == 0
				&&  reqMsg.getMetaTransferEncoding() == NONE_ENCODING) {
					reqMsg.setStatus(REQ_DONE);
					return NONE_STATUS_CODE;
				}
				break ;
			}
		} else {//데이터가 있는 줄 처리
			EnumStatusCode statusCode = this->handleOneLine(readBuffer.substr(cursorFront, cursorBack-cursorFront-2), reqMsg);
			if (statusCode != NONE_STATUS_CODE)
				return statusCode;
		}
	}

	// Body를 처리해야하는 순서에서 readBuffer에 사용가능한 데이터가 없음
	if (!readBuffer.size())
		return NONE_STATUS_CODE;

	// 1-2. Body 처리, 청크전송인지 아닌지에 따라 처리과정을 달리함
	if (reqMsg.getMetaTransferEncoding() == CHUNK)
		return this->cleanUpChunkedBody(readBuffer, reqMsg);
	else
		return this->parseBody(readBuffer, reqMsg);
}

// 1-1-2번에서 \r\n으로 구분된 줄, 현 메시지 상태별로 의미해석하여 파싱하는 함수
EnumStatusCode RequestParser::handleOneLine(const std::string &line, RequestMessage &reqMsg) {
	const EnumReqStatus curStatus = reqMsg.getStatus();

	if (curStatus == REQ_INIT
	||  curStatus == REQ_TOP_CRLF)
		return this->parseStartLine(line, reqMsg);
	if (curStatus == REQ_STARTLINE
	||  curStatus == REQ_HEADER_FIELD)
		return this->parseFieldLine(line, reqMsg);
	return NONE_STATUS_CODE;
}

// 1-1-2번에서 \r\n으로만 구성된 줄, 현재 RequestMessage 상태에서 CRLF줄이 유효한지 검증하고, 다음 상태를 지정해주는 함수
EnumReqStatus RequestParser::handleCRLFLine(const EnumReqStatus &curStatus) {
	// 유효한 것으로 처리되는 두 위치의 CRLF외에는 전부 에러
	if (curStatus == REQ_INIT)
		return REQ_TOP_CRLF;
	if (curStatus == REQ_HEADER_FIELD)
		return REQ_HEADER_CRLF;
	return REQ_ERROR;
}
