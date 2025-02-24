#include "RequestParser.hpp"
#include "utils.hpp"
#include "GlobalConfig.hpp"
#include <stdexcept>
#include <sstream>

RequestParser::RequestParser() : oneLineMaxLength_(ONELINE_MAX_LENGTH), uriMaxLength_(URI_MAX_LENGTH), bodyMaxLength_(BODY_MAX_LENGTH) {}
RequestParser::~RequestParser() {}

void RequestParser::setConfigBodyLength(size_t length) {
	this->bodyMaxLength_ = length;
}

// 1. 기본 파싱 로직 함수
// readData: recv함수로 읽은 요쳥 데이터
// readBuffer: 기존 요청데이터를 파싱하고 남은 데이터
// reqMsg: 현재 요청 데이터를 파싱하고 저장할 RequestMessage
int RequestParser::parse(const std::string &readData, std::string &readBuffer, RequestMessage &reqMsg) {
	std::istringstream iss(readData + readBuffer);
	std::string buffer;
	EnumReqStatus status = reqMsg.getStatus();
	
	if (status != REQ_HEADER_CRLF // 1-1. Body가 아닌, start-line이나 field-line인 경우
	&&  status != REQ_BODY) {
		while (std::getline(iss, buffer, '\n')) {
			if (iss.eof()) {// 1-1-1. \n이 나오지 않고 readData가 끝난 상태. 다음 loop로 넘어감
				readBuffer = buffer;
				return NONE_STATUS_CODE;
			} else if (!buffer.empty() && buffer[buffer.size() - 1] == '\r') {// 1-1-2. \r\n 줄 파싱
				if (buffer.size() == 1) {
					status = this->handleCRLFLine(reqMsg.getStatus());
					reqMsg.setStatus(status);
					if (status == REQ_HEADER_CRLF) {
						std:getline(iss, buffer, '\0');
						readBuffer = buffer;
						return NONE_STATUS_CODE;
					}
					if (status == REQ_ERROR)
						return 400;//status code: 유효하지 않은 CRLF 위치
				} else {
					int statusCode = this->handleOneLine(buffer.erase(buffer.size() - 1), reqMsg);
					if (statusCode != NONE_STATUS_CODE)
						return statusCode;
				}
			} else {//1-1-3. \r\n이 아닌 단일 개행인 경우 에러
				return 400;//status code: CRLF가 아닌, 단일 LF
			}
		}
	} else {// 1-2. Body인 경우
		buffer = readData;
	}

	// 1-3. Body 처리, 청크전송인지 아닌지에 따라 처리과정을 달리함
	if (reqMsg.getMetaTransferEncoding() == CHUNK)
		return this->cleanUpChunkedBody(buffer, readBuffer, reqMsg);
	else
		return this->parseBody(buffer, readBuffer, reqMsg);
}

// 1-1-2번에서 \r\n으로 구분된 줄, 현 메시지 상태별로 의미해석하여 파싱하는 함수
int RequestParser::handleOneLine(const std::string &line, RequestMessage &reqMsg) {
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
		return REQ_HEADER_CRLF;// MUST TO DO: 이 상태에서 request가 새롭게 들어오면 DONE을 해줘야하는 상황이 있음
	return REQ_ERROR;
	// MUST TO DO: 이상태에서 DONE의 판단을 할 수 있는가?
}

// 2. Start-line 한 줄을 파싱하는 함수
// line: \r\n기준으로 나뉜 요청 데이터의 첫 줄
// reqMsg: 현재 요청 데이터를 파싱하고 저장할 RequestMessage
int RequestParser::parseStartLine(const std::string &line, RequestMessage &reqMsg) {
	EnumReqStatus status = reqMsg.getStatus();
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
			return 501;//status code: 구현되지 않은 메서드
		}
	}

	if (std::getline(iss, buffer, ' '))
		reqMsg.setTargetURI(buffer);
	else
		return 400;//status code: "URI version" 형식이 유효하지 않음
	//MUST TO DO: URI길이 제한 

	iss >> buffer;
	if (buffer == "HTTP/1.1")
		reqMsg.setStatus(REQ_STARTLINE);
	else {
		reqMsg.setStatus(REQ_ERROR);
		return 400;//status code: HTTP 버전 오류
	}
	return NONE_STATUS_CODE;
}

// 3. field-line 한 줄을 파싱하는 함수
// line: \r\n기준으로 나뉜 요청 데이터, 첫 줄(start line)이 아닌 다음 줄
// reqMsg: 현재 요청 데이터를 파싱하고 저장할 RequestMessage
int RequestParser::parseFieldLine(const std::string &line, RequestMessage &reqMsg) {
	std::istringstream iss(line);
	std::string name;
	std::string value;
	std::vector<std::string> values;

	// 3-1. field-line의 기본 파싱 (name: value, ...)
	std::getline(iss, name, ':');
	while (std::getline(iss, value, ',')) {
		value = utils::strtrim(value);
		values.push_back(value);
	}
	// map<string, vector> 형태의 멤버변수 데이터에 저장
	reqMsg.addFieldLine(name, values);
	
	// 3-2. field value 갯수 검증
	if (validateFieldValueCount(name, values.size()))
		return 400;
	
	// 3-3. field value중 RequestMessage의 메타데이터 처리
	if (handleFieldValue(name, value, reqMsg))
		return 400;
		
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
	if (name == "Host") {//									1) Host
		reqMsg.setMetaHost(value);
	} else if (name == "Connection") {//					2) Connection
		EnumConnect connection;
		if (value == "keep-alive")
			connection = KEEP_ALIVE;
		else if (value == "close")
			connection = CLOSE;
		else
			return false;
		reqMsg.setMetaConnection(connection);
	} else if (name == "Content-Length") {//				3) Content-Length
		size_t contentLength;
		try {
			contentLength = utils::sto_size_t(value);
		} catch (const std::exception & e) {
			return false;
		}
		reqMsg.setMetaContentLength(contentLength);
	} else if (name == "Transfer-Encoding") {//				4) Transfer-Encoding
		EnumTransEnc transferEncoding;
		if (value == "chunked")
			transferEncoding = CHUNK;
		else
			return false;
		reqMsg.setMetaTransferEncoding(transferEncoding);
	} else if (name == "Content-Type") {//					5) Content-Type
		reqMsg.setMetaContentType(value);
	}
	return true;
}

// 4. Body를 파싱하는 함수, Content-Length의 길이에 따라 예외 처리됨. 유효한 문자열은 기존의 Body의 추가함
// data: Body로 파싱할 요청 데이터 
// readBuffer: 남은 데이터를 저장할 ClientSession의 readBuffer
// reqMsg: 현재 요청 데이터를 파싱하고 저장할 RequestMessage
int RequestParser::parseBody(const std::string &data, std::string &readBuffer, RequestMessage &reqMsg) {
	const size_t contentLength = reqMsg.getMetaContentLength();

	// 4-1. 최종 body의 길이 >= 현재 저장된 body길이 + 파싱하려는 길이
	if (contentLength >= reqMsg.getBodyLength() + data.length()) {
		if (this->bodyMaxLength_ < reqMsg.getBodyLength() + data.length())
			return 413;//status code: Body가 설정보다 큼 “Request Entity Too Large”

		reqMsg.addBody(data);
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
	reqMsg.addBody(data.substr(0, substrSize));
	std::string remainData = data.substr(substrSize);
	
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
		return 400;//status code: 의심스러운 Body 거부
	}
	reqMsg.setStatus(REQ_DONE);
	readBuffer = remainData;
	return NONE_STATUS_CODE;
}

// 5. 청크 전송 파싱하는 함수. Transfer-Encoding이 chunked여서 Body가 청크로 아루어진 경우
// data: Body로 파싱할 요청 데이터
// readBuffer: 남은 데이터를 저장할 ClientSession의 readBuffer
// reqMsg: 현재 요청 데이터를 파싱하고 저장할 RequestMessage
int RequestParser::cleanUpChunkedBody(const std::string &data, std::string &readBuffer, RequestMessage &reqMsg) {
	std::istringstream iss(data);
	std::string buffer;
	std::string tmp;
	size_t chunkSize;

	while (std::getline(iss, buffer, '\n')) {
		if (iss.eof()) {// 5-1. \n으로 getline되지 않은 경우 readBuffer에 남겨 다음 loop에 진행
			readBuffer = buffer;
			return NONE_STATUS_CODE;
		}
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
			if (endPos - currentPos < chunkSize + 2) {
				readBuffer = tmp + iss.str().substr(static_cast<std::string::size_type>(currentPos));
				return NONE_STATUS_CODE;
			}

			// 파싱한 청크사이즈 기준으로 청크 데이터를 읽음
			buffer.resize(chunkSize + 2);
			iss.read(&buffer[0], chunkSize + 2);

			// 청크 데이터가 \r\n형식에 맞춰 들어왔는지 검증
			if (buffer.compare(buffer.size() - 2, 2, "\r\n") != 0)
				return 400;//status code: 유효하지 않은 청크 데이터 형식
				
			if (this->bodyMaxLength_ < reqMsg.getBodyLength() + chunkSize)
				return 413;//status code: Body가 설정보다 큼 “Request Entity Too Large”

				// 청크 데이터에 \r\n를 제거하여 Body에 저장
			reqMsg.addBody(buffer.substr(0, buffer.size() - 2));
		} else { //\r\n으로 이루어져있지 않음
			return 400;//status code: CRLF가 아닌, 단일 LF
		}
	}
}
