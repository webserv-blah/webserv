#include "RequestParser.hpp"
#include "../utils/utils.hpp"
#include "../GlobalConfig/GlobalConfig.hpp"
#include "../ClientSession/ClientSession.hpp"
#include "../include/errorUtils.hpp"
#include <stdexcept>
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
	DEBUG_LOG("[RequestParser] Starting parse for client fd:" << curSession.getClientFd());
	
	// 요청 데이터가 비어있는 경우 경고 로그 출력
	if (readData.empty()) {
		DEBUG_LOG("[RequestParser] Warning: Empty request data received");
	} else {
		DEBUG_LOG("[RequestParser] Request data first line: " << readData.substr(0, readData.find("\n")));
	}
	
	// 새로운 요청일 때, RequestMessage 동적할당
	if (curSession.getReqMsgPtr() == NULL) {
		DEBUG_LOG("[RequestParser] Creating new RequestMessage for client fd:" << curSession.getClientFd());
		try {
			curSession.setReqMsg(new RequestMessage());// 이후 요청처리(handler&builder) 완료 후, delete 필요 (ClientSession.resetRequest()메서드 사용)
		} catch (const std::exception& e) {
			DEBUG_LOG("[RequestParser] Failed to create RequestMessage: " << e.what());
			return INTERNAL_SERVER_ERROR;
		}
		
		// 설정이 없으면 기본 서버 설정 배정
		if (curSession.getConfigPtr() == NULL) {
			DEBUG_LOG("[RequestParser] No config set for client, using default server config");
			// 기본 서버 설정 가져오기
			const GlobalConfig &globalConfig = GlobalConfig::getInstance();
			const RequestConfig* defaultConfig = globalConfig.findRequestConfig(curSession.getListenFd(), "", "");
			if (defaultConfig != NULL) {
				curSession.setConfig(defaultConfig);
				DEBUG_LOG("[RequestParser] Default config set successfully");
			} else {
				DEBUG_LOG("[RequestParser] Failed to find default config, this may cause issues");
			}
		}
	}

	RequestMessage &reqMsg = curSession.accessReqMsg();
	std::string &readBuffer = curSession.accessReadBuffer();
	EnumReqStatus status = reqMsg.getStatus();

	size_t cursorFront = readBuffer.size();
	size_t cursorBack = readBuffer.size();
	
	readBuffer.append(readData);
	DEBUG_LOG("[RequestParser] Appended " << readData.size() << " bytes to read buffer, total size: " << readBuffer.size());
	
	// 1-1. Body가 아닌, start-line이나 field-line인 경우
	// 무한 루프 방지를 위한 안전장치
	int maxLoops = 50; // 최대 반복 횟수 제한
	int loopCount = 0;
	
	while (status != REQ_HEADER_CRLF && status != REQ_BODY && loopCount < maxLoops) {
		loopCount++;
		// 버퍼 범위를 벗어나지 않도록 시작 위치 조정
		size_t startPos = (cursorBack + 2 < readBuffer.size()) ? cursorBack + 2 : 0;
		size_t findResult = readBuffer.find(CRLF, startPos, 2);
		DEBUG_LOG("[RequestParser] CRLF 검색 위치: " << startPos << ", 버퍼 크기: " << readBuffer.size());

		// 첫 요청인 경우 특별 처리
		if (status == REQ_INIT) {
			// GET, POST, DELETE 요청 감지
			if (readBuffer.find("GET", 0) == 0 || 
				readBuffer.find("POST", 0) == 0 || 
				readBuffer.find("DELETE", 0) == 0) {
				DEBUG_LOG("[RequestParser] Detected HTTP request start");
				// 첫 줄에서만 CRLF를 찾도록 수정 (cursorBack이 0일 때)
				if (cursorBack == 0) {
					findResult = readBuffer.find(CRLF, 0, 2);
					if (findResult == std::string::npos) {
						// CRLF가 없으면 LF만 있는지 확인
						findResult = readBuffer.find(LF, 0, 1);
					}
				}
			}
		}

		// find결과에 따라 cursor(인덱스 파싱)을 할지 다음 recv를 기다릴지 결정
		if (findResult != std::string::npos) {
			cursorFront = (cursorBack == 0) ? 0 : cursorBack+2;
			cursorBack = findResult;
			DEBUG_LOG("[RequestParser] Parsing line: cursorFront=" << cursorFront << ", cursorBack=" << cursorBack);
		} else {// \n이 나오지 않고 readData가 끝난 상태. 다음 loop로 넘어감
			// 단일 LF는 HTTP/1.1 스펙에서는 오류지만, 일부 클라이언트는 이를 보내기도 함
			// 그러므로 단일 LF가 있는지 확인하고, 있다면 조금 더 관대하게 처리
			size_t lfPos = readBuffer.find(LF, startPos);
			if (lfPos != std::string::npos) {
				DEBUG_LOG("[RequestParser] Found single LF at position " << lfPos << ", treating as line end");
				cursorFront = (cursorBack == 0) ? 0 : cursorBack+1;
				cursorBack = lfPos;
			} else {
				DEBUG_LOG("[RequestParser] No line ending found, waiting for more data");
				return NONE_STATUS_CODE;
			}
		}
		
		// CRLF줄 처리
		if (cursorBack == cursorFront) {
			status = this->handleCRLFLine(reqMsg.getStatus());
			reqMsg.setStatus(status);
			// 올바르지 않은 CRLF줄 에러
			if (status == REQ_ERROR)
				return BAD_REQUEST;//status code: 유효하지 않은 CRLF 위치
			// field-line까지 다 읽은 후, 요청메시지에 맞는 RequestConfig를 설정하고 Host헤더필드 유무를 검증함
			if (status == REQ_HEADER_CRLF) {
				// ClientSession에 Config설정
				const GlobalConfig &globalConfig = GlobalConfig::getInstance();
				curSession.setConfig(globalConfig.findRequestConfig(curSession.getListenFd(), reqMsg.getMetaHost(), reqMsg.getTargetURI()));

				// 헤더필드 검증 및 처리
				// 1) Host 헤더가 비어있으면 기본값 사용 (일부 클라이언트는 Host 헤더를 보내지 않을 수 있음)
				DEBUG_LOG("[RequestParser] Host header check - current value: '" << reqMsg.getMetaHost() << "'");
				if (reqMsg.getMetaHost().empty()) {
					DEBUG_LOG("[RequestParser] No Host header found, using default host");
					reqMsg.setMetaHost("localhost"); // 기본값으로 localhost 설정
				} else {
					DEBUG_LOG("[RequestParser] Using existing Host header: " << reqMsg.getMetaHost());
				}
				// 2) Body size가 정해진 것이 없을때, 종료 처리
				if (reqMsg.getMetaContentLength() == 0
				&&  reqMsg.getMetaTransferEncoding() == NONE_ENCODING) {
					reqMsg.setStatus(REQ_DONE);
					readBuffer.erase(0, cursorBack+2);
					return NONE_STATUS_CODE;
				}
				break ;
			}
		} else {//데이터가 있는 줄 처리
			EnumStatusCode statusCode = this->handleOneLine(readBuffer.substr(cursorFront, cursorBack-cursorFront+2), reqMsg);
			if (statusCode != NONE_STATUS_CODE)
				return statusCode;
		}
	}

	// 안전장치: 최대 반복 횟수를 초과한 경우 에러 처리
	if (loopCount >= maxLoops) {
		DEBUG_LOG("[RequestParser] Maximum loop count exceeded (" << maxLoops << "), possible infinite loop detected");
		DEBUG_LOG("[RequestParser] Current buffer content (first 100 chars): " << 
				 readBuffer.substr(0, std::min(readBuffer.size(), static_cast<size_t>(100))));
		reqMsg.setStatus(REQ_ERROR);
		return BAD_REQUEST;
	}
	
	// 상태 디버깅을 위한 추가 로그
	DEBUG_LOG("[RequestParser] After parsing loop - status: " << status << 
			 ", buffer size: " << readBuffer.size() << 
			 ", loopCount: " << loopCount);

	// Body를 처리해야하는 순서에서 readBuffer에 사용가능한 데이터가 없음
	if (cursorBack+1 == readBuffer.size()) {
		readBuffer = "";
		return NONE_STATUS_CODE;
	}

	readBuffer.erase(0, cursorBack+2);

	// 1-2. Body 처리, 청크전송인지 아닌지에 따라 처리과정을 달리함
	if (reqMsg.getMetaTransferEncoding() == CHUNK)
		return this->cleanUpChunkedBody(readBuffer, reqMsg);
	else
		return this->parseBody(readBuffer, reqMsg);
}

// 1-1-2번에서 \r\n으로 구분된 줄, 현 메시지 상태별로 의미해석하여 파싱하는 함수
EnumStatusCode RequestParser::handleOneLine(const std::string &line, RequestMessage &reqMsg) {
	const EnumReqStatus curStatus = reqMsg.getStatus();

	// 디버그를 위한 로그 추가
	DEBUG_LOG("[RequestParser] Handling line with status: " << curStatus << ", line length: " << line.length());
	
	// 비어있는 라인은 무시
	if (line.empty() || line == CRLF) {
		DEBUG_LOG("[RequestParser] Empty line or just CRLF, ignoring");
		return NONE_STATUS_CODE;
	}

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
	DEBUG_LOG("[RequestParser] Handling CRLF line with status: " << curStatus);
	
	// 모든 상태에서 더 관대하게 처리 - CRLF는 여러 위치에서 허용
	if (curStatus == REQ_INIT) {
		DEBUG_LOG("[RequestParser] CRLF at beginning of request, moving to TOP_CRLF state");
		return REQ_TOP_CRLF;
	}
	else if (curStatus == REQ_STARTLINE) {
		// 시작 줄 바로 다음에 CRLF가 있는 경우, 헤더가 없는 요청으로 간주
		DEBUG_LOG("[RequestParser] CRLF after start line with no headers, treating as header end");
		return REQ_HEADER_CRLF;
	}
	else if (curStatus == REQ_HEADER_FIELD) {
		DEBUG_LOG("[RequestParser] CRLF after header fields, headers complete");
		return REQ_HEADER_CRLF; // 헤더 필드 다음의 CRLF는 헤더의 끝을 의미
	}
	else if (curStatus == REQ_TOP_CRLF) {
		// 연속된 CRLF도 허용
		DEBUG_LOG("[RequestParser] Multiple CRLFs at beginning of request, ignoring");
		return REQ_TOP_CRLF;
	}
	else if (curStatus == REQ_HEADER_CRLF) {
		// 헤더 끝 이후의 추가 CRLF도 허용
		DEBUG_LOG("[RequestParser] Additional CRLF after headers, ignoring");
		return REQ_HEADER_CRLF;
	}
	else if (curStatus == REQ_BODY) {
		// 바디에서 CRLF는 바디의 일부일 수 있음
		DEBUG_LOG("[RequestParser] CRLF in body, treating as part of body");
		return REQ_BODY;
	}
	else if (curStatus == REQ_DONE) {
		// 요청 완료 후의 CRLF는 무시
		DEBUG_LOG("[RequestParser] CRLF after request done, ignoring");
		return REQ_DONE;
	}
	
	// 그 외의 경우는 에러처리 대신 경고만 로깅
	DEBUG_LOG("[RequestParser] CRLF in unexpected location, status: " << curStatus << ", but continuing anyway");
	return curStatus; // 현재 상태 유지
}

// 2. Start-line 한 줄을 파싱하는 함수
// line: \r\n기준으로 나뉜 요청 데이터의 첫 줄
// reqMsg: 현재 요청 데이터를 파싱하고 저장할 RequestMessage
EnumStatusCode RequestParser::parseStartLine(const std::string &line, RequestMessage &reqMsg) {
	DEBUG_LOG("[RequestParser] Parsing start line: " << line);

	// 줄이 비어있거나 CRLF만 있는 경우 처리
	if (line.empty() || line == CRLF) {
		return NONE_STATUS_CODE;
	}

	std::istringstream iss(line);
	std::string buffer;

	// 메서드 파싱
	if (std::getline(iss, buffer, ' ')) {
		DEBUG_LOG("[RequestParser] Method: " << buffer);
		if (buffer == "GET")
			reqMsg.setMethod(GET);
		else if (buffer == "POST")
			reqMsg.setMethod(POST);
		else if (buffer == "DELETE")
			reqMsg.setMethod(DELETE);
		else {
			DEBUG_LOG("[RequestParser] Unsupported method: " << buffer);
			reqMsg.setStatus(REQ_ERROR);
			return NOT_IMPLEMENTED;//status code: 구현되지 않은 메서드
		}
	} else {
		DEBUG_LOG("[RequestParser] Failed to parse method");
		return BAD_REQUEST;
	}

	// URI 파싱
	if (std::getline(iss, buffer, ' ')) {
		DEBUG_LOG("[RequestParser] URI: " << buffer);
		if (buffer.size() > this->uriMaxLength_) {
			DEBUG_LOG("[RequestParser] URI too long: " << buffer.size() << " > " << this->uriMaxLength_);
			return URI_TOO_LONG;//status code: URI가 서버지원사이즈보다 큼
		}
		reqMsg.setTargetURI(buffer);
	} else {
		DEBUG_LOG("[RequestParser] Failed to parse URI");
		return BAD_REQUEST;//status code: start-line의 "method URI version" 형식이 유효하지 않음
	}

	// HTTP 버전 파싱
	if (std::getline(iss, buffer)) {
		// 버전 문자열에서 앞뒤 공백 제거
		if (buffer.find_first_not_of(" \t") != std::string::npos) {
			buffer.erase(0, buffer.find_first_not_of(" \t"));
		}
		if (buffer.find_last_not_of(" \t\r\n") != std::string::npos) {
			buffer.erase(buffer.find_last_not_of(" \t\r\n") + 1);
		}
		DEBUG_LOG("[RequestParser] HTTP version: '" << buffer << "'");
		
		// HTTP 버전 문자열 단어 단위로 출력
		for (size_t i = 0; i < buffer.length(); i++) {
			DEBUG_LOG("[RequestParser] HTTP version char at " << i << ": '" << buffer[i] << "' (ASCII: " << static_cast<int>(buffer[i]) << ")");
		}
		
		// \r\n 등의 특수 문자 검사
		if (buffer.find('\r') != std::string::npos || buffer.find('\n') != std::string::npos) {
			DEBUG_LOG("[RequestParser] HTTP version contains CR or LF, cleaning up");
			buffer.erase(std::remove(buffer.begin(), buffer.end(), '\r'), buffer.end());
			buffer.erase(std::remove(buffer.begin(), buffer.end(), '\n'), buffer.end());
		}
		
		// 대소문자 구분 없이 비교
		std::string lowerBuffer = buffer;
		std::transform(lowerBuffer.begin(), lowerBuffer.end(), lowerBuffer.begin(), ::tolower);
		
		if (buffer == "HTTP/1.1" || lowerBuffer == "http/1.1") {
			reqMsg.setStatus(REQ_STARTLINE);
			DEBUG_LOG("[RequestParser] Valid HTTP/1.1 version detected");
		} else {
			DEBUG_LOG("[RequestParser] Unsupported HTTP version: '" << buffer << "'");
			reqMsg.setStatus(REQ_ERROR);
			return BAD_REQUEST;//status code: HTTP 버전 오류
		}
	} else {
		DEBUG_LOG("[RequestParser] Failed to parse HTTP version, no content after URI");
		return BAD_REQUEST;
	}
	
	return NONE_STATUS_CODE;
}

// 3. field-line 한 줄을 파싱하는 함수
// line: \r\n기준으로 나뉜 요청 데이터, 첫 줄(start line)이 아닌 다음 줄
// reqMsg: 현재 요청 데이터를 파싱하고 저장할 RequestMessage
EnumStatusCode RequestParser::parseFieldLine(const std::string &line, RequestMessage &reqMsg) {
	DEBUG_LOG("[RequestParser] Parsing field line: '" << line << "'");
	
	// 빈 줄이나 공백만 있는 줄은 무시
	if (line.empty() || line == "\r\n" || line == "\n") {
		DEBUG_LOG("[RequestParser] Empty field line, ignoring");
		return NONE_STATUS_CODE;
	}
	
	std::istringstream iss(line);
	std::string name;
	std::string value;
	std::vector<std::string> values;

	// 3-1. field-line의 기본 파싱 (name: value, ...)
	if (!std::getline(iss, name, ':')) {
		DEBUG_LOG("[RequestParser] Failed to parse header name, no colon in line");
		return BAD_REQUEST;
	}
	
	// 헤더 이름 공백 제거
	name = utils::strtrim(name);
	DEBUG_LOG("[RequestParser] Header name: '" << name << "'");
	
	// 값 읽기 (쉼표로 구분된 여러 값 처리)
	while (std::getline(iss, value, ',')) {
		value = utils::strtrim(value);
		if (!value.empty()) {
			values.push_back(value);
			DEBUG_LOG("[RequestParser] Header value: '" << value << "'");
		}
	}
	
	// 값이 비어있는 경우 처리
	if (values.empty()) {
		DEBUG_LOG("[RequestParser] Header has no values, adding empty value");
		values.push_back("");
	}
	
	// map<string, vector> 형태의 멤버변수 데이터에 저장
	reqMsg.addFieldLine(name, values);
	
	// 3-2. field value 갯수 검증 - 더 관대하게 처리
	if (!validateFieldValueCount(name, values.size())) {
		DEBUG_LOG("[RequestParser] Invalid field value count for header: " << name);
		// 에러 대신 경고만 출력하고 계속 진행
		// return BAD_REQUEST;
	}
	
	// 3-3. field value중 RequestMessage의 메타데이터 처리
	if (!handleFieldValue(name, values[0], reqMsg)) {
		DEBUG_LOG("[RequestParser] Failed to handle field value for header: " << name);
		// 에러 대신 경고만 출력하고 계속 진행
		// return BAD_REQUEST;
	}
	
	// 3-4. RequestMessage가 하나 이상의 field-line를 갖고 있으며, 아직 CRLF가 나오지 않음을 뜻함
	reqMsg.setStatus(REQ_HEADER_FIELD);
	return NONE_STATUS_CODE;
}

// 3-2. field-line의 value 갯수를 검증하는 함수
bool RequestParser::validateFieldValueCount(const std::string &name, const int count) {
	if (count < 1)
		return false;
	if (count > 1
	&& (name == "Host" || name == "Content-Length"
	||  name == "Connection" || name == "Transfer-Encoding"
	||  name == "User-Agent" ||  name == "Authorization"
	||  name == "Referer" ||  name == "Range"
	||  name == "If-Modified-Since" ||  name == "If-Unmodified-Since"
	||  name == "If-Match" ||  name == "If-None-Match"
	||  name == "Content-Location"))
		return false;
	return true;
}

// 3-3. field-line의 value중 RequestMessage의 메타데이터에 해당하는 value 파싱하는 함수
bool RequestParser::handleFieldValue(const std::string &name, const std::string &value, RequestMessage &reqMsg) {
	DEBUG_LOG("[RequestParser] Handling field value for: " << name << "=" << value);
	
	// 대소문자 구분 없이 비교하기 위한 소문자 변환
	std::string lowerName = name;
	std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
	std::string lowerValue = value;
	std::transform(lowerValue.begin(), lowerValue.end(), lowerValue.begin(), ::tolower);
	
	if (lowerName == "host") {//									1) Host
		DEBUG_LOG("[RequestParser] Setting Host metadata: " << value);
		reqMsg.setMetaHost(value);
	} else if (lowerName == "connection") {//					2) Connection
		EnumConnect connection = KEEP_ALIVE; // 기본값
		if (lowerValue == "keep-alive") {
			connection = KEEP_ALIVE;
			DEBUG_LOG("[RequestParser] Setting Connection metadata: KEEP_ALIVE");
		} else if (lowerValue == "close") {
			connection = CLOSE;
			DEBUG_LOG("[RequestParser] Setting Connection metadata: CLOSE");
		} else {
			DEBUG_LOG("[RequestParser] Unknown Connection value: " << value << ", using default KEEP_ALIVE");
		}
		reqMsg.setMetaConnection(connection);
	} else if (lowerName == "content-length") {//				3) Content-Length
		size_t contentLength = 0;
		try {
			contentLength = utils::sto_size_t(value);
			DEBUG_LOG("[RequestParser] Setting Content-Length metadata: " << contentLength);
			reqMsg.setMetaContentLength(contentLength);
		} catch (const std::exception & e) {
			DEBUG_LOG("[RequestParser] Invalid Content-Length value: " << value << ", error: " << e.what());
			return false;
		}
	} else if (lowerName == "transfer-encoding") {//				4) Transfer-Encoding
		EnumTransEnc transferEncoding = NONE_ENCODING; // 기본값
		if (lowerValue == "chunked") {
			transferEncoding = CHUNK;
			DEBUG_LOG("[RequestParser] Setting Transfer-Encoding metadata: CHUNK");
		} else {
			DEBUG_LOG("[RequestParser] Unknown Transfer-Encoding value: " << value << ", using default NONE_ENCODING");
		}
		reqMsg.setMetaTransferEncoding(transferEncoding);
	} else if (lowerName == "content-type") {//					5) Content-Type
		DEBUG_LOG("[RequestParser] Setting Content-Type metadata: " << value);
		reqMsg.setMetaContentType(value);
	} else {
		DEBUG_LOG("[RequestParser] Header not used for metadata: " << name);
	}
	return true;
}

// 4. Body를 파싱하는 함수, Content-Length의 길이에 따라 예외 처리됨. 유효한 문자열은 기존의 Body의 추가함
// readBuffer: Body로 파싱할 요청 데이터이자, 남은 데이터를 저장할 ClientSession의 readBuffer
// reqMsg: 현재 요청 데이터를 파싱하고 저장할 RequestMessage
EnumStatusCode RequestParser::parseBody(std::string &readBuffer, RequestMessage &reqMsg) {
	DEBUG_LOG("[RequestParser] Parsing body, content length:" << reqMsg.getMetaContentLength() << ", current body size:" << reqMsg.getBodyLength());
	const size_t contentLength = reqMsg.getMetaContentLength();

	// 4-1. 최종 body의 길이 >= 현재 저장된 body길이 + 파싱하려는 길이
	if (contentLength >= reqMsg.getBodyLength() + readBuffer.length()) {
		if (this->bodyMaxLength_ < reqMsg.getBodyLength() + readBuffer.length())
			return CONTENT_TOO_LARGE;//status code: Body가 설정보다 큼 “Request Entity Too Large”

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
	std::istringstream iss(readBuffer);
	std::string buffer;
	std::string tmp;
	size_t chunkSize;

	while (std::getline(iss, buffer, '\n')) {
		if (iss.eof())// 5-1. \n으로 getline되지 않은 경우 readBuffer에 남겨 다음 loop에 진행
			return NONE_STATUS_CODE;
		if (!buffer.empty() && buffer[buffer.size() - 1] == '\r') {// 5-2. \r\n으로 찾은 줄 파싱
			// iss에 남은 길이 측정을 위한 도구
			std::streampos currentPos = iss.tellg();
			iss.seekg(0, std::ios::end);
			std::streampos endPos = iss.tellg();
			iss.seekg(currentPos);

			// 청크 사이즈 파싱
			chunkSize = utils::sto_size_t(buffer.erase(buffer.size() - 1));
			if (chunkSize == 0) {
				reqMsg.setStatus(REQ_DONE);
				reqMsg.setMetaContentLength(reqMsg.getBodyLength());
				readBuffer = iss.str().substr(static_cast<std::string::size_type>(currentPos)); 
				return NONE_STATUS_CODE;
			}
			
			// 읽어낸 데이터가 모자를 경우 readBuffer에 돌려놓기 위함
			tmp = buffer + "\r\n";
			if (static_cast<size_t>(endPos - currentPos) < chunkSize + 2) {
				readBuffer = tmp + iss.str().substr(static_cast<std::string::size_type>(currentPos));
				return NONE_STATUS_CODE;
			}

			// 파싱한 청크사이즈 기준으로 청크 데이터를 읽음
			buffer.resize(chunkSize + 2);
			iss.read(&buffer[0], chunkSize + 2);

			// 청크 데이터가 \r\n형식에 맞춰 들어왔는지 검증
			if (buffer.compare(buffer.size() - 2, 2, "\r\n") != 0)
				return BAD_REQUEST;//status code: 유효하지 않은 청크 데이터 형식
				
			if (this->bodyMaxLength_ < reqMsg.getBodyLength() + chunkSize)
				return CONTENT_TOO_LARGE;//status code: Body가 설정보다 큼 “Request Entity Too Large”

				// 청크 데이터에 \r\n를 제거하여 Body에 저장
			reqMsg.addBody(buffer.substr(0, buffer.size() - 2));
		} else { //\r\n으로 이루어져있지 않음
			return BAD_REQUEST;//status code: CRLF가 아닌, 단일 LF
		}
	}
	readBuffer = "";
	return NONE_STATUS_CODE;
}
