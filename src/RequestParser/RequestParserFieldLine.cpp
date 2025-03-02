#include "RequestParser.hpp"
#include "../utils/utils.hpp"
#include "../include/errorUtils.hpp"
#include <sstream>

// 3. field-line 한 줄을 파싱하는 함수
// line: \r\n기준으로 나뉜 요청 데이터, 첫 줄(start line)이 아닌 다음 줄
// reqMsg: 현재 요청 데이터를 파싱하고 저장할 RequestMessage
EnumStatusCode RequestParser::parseFieldLine(const std::string &line, RequestMessage &reqMsg) {
	std::istringstream iss(line);
	std::string name;
	std::string value;
	std::vector<std::string> values;

	// 3-1. field-line의 기본 파싱 (name: value, ...)
	std::getline(iss, name, ':');
	if (name == "Host" || name == "Content-Length"
	||  name == "Connection" || name == "Transfer-Encoding"
	||  name == "User-Agent" ||  name == "Authorization"
	||  name == "If-Modified-Since" ||  name == "If-Unmodified-Since"
	||  name == "Referer" ||  name == "Content-Location") {
		// 단일 헤더인데, 또 반복하여 들어온 경우
		if (reqMsg.getFields().count(name))
			return BAD_REQUEST;

		std::getline(iss, value, '\0');
		values.push_back(utils::strtrim(value));
	} else {
		while (std::getline(iss, value, ',')) {
			value = utils::strtrim(value);
			values.push_back(value);
		}
	}

	// 3-2. field value 갯수 검증
	if (values.size() == 0) {
		webserv::logError(ERROR, "BAD_REQUEST", 
			"invalid field(" + name + ") value counts",  
			"RequestParser::parseFieldLine");
		return BAD_REQUEST;
	}
		

	// 3-3. field value중 RequestMessage의 메타데이터 처리
	if (!handleFieldValue(name, values[0], reqMsg))
		return BAD_REQUEST;
	
	// map<string, vector> 형태의 멤버변수 데이터에 저장
	reqMsg.addFieldLine(name, values);

	// 3-4. RequestMessage가 하나 이상의 field-line를 갖고 있으며, 아직 CRLF가 나오지 않음을 뜻함
	reqMsg.setStatus(REQ_HEADER_FIELD);

	return NONE_STATUS_CODE;
}

// 3-3. field-line의 value중 RequestMessage의 메타데이터에 해당하는 value 파싱하는 함수
bool RequestParser::handleFieldValue(const std::string &name, const std::string &value, RequestMessage &reqMsg) {
	if (name == "Host") {//									1) Host
		if (value.find("/", 0) != std::string::npos) {
			webserv::logError(ERROR, "BAD_REQUEST", 
				"Host meta data error", 
				"RequestParser::handleFieldValue");
			return false;
		}
		reqMsg.setMetaHost(value);
	} else if (name == "Connection") {//					2) Connection
		EnumConnect connection;
		if (value == "keep-alive")
			connection = KEEP_ALIVE;
		else if (value == "close")
			connection = CLOSE;
		else {
			webserv::logError(ERROR, "BAD_REQUEST", 
				"Connection meta data error", 
				"RequestParser::handleFieldValue");
			return false;
		}
		reqMsg.setMetaConnection(connection);
	} else if (name == "Content-Length") {//				3) Content-Length
		size_t contentLength;
		try {
			contentLength = utils::sto_size_t(value);
		} catch (const std::exception & e) {
			webserv::logError(ERROR, "BAD_REQUEST", 
				"Content Length meta data error", 
				"RequestParser::handleFieldValue");
			return false;
		}
		reqMsg.setMetaContentLength(contentLength);
	} else if (name == "Transfer-Encoding") {//				4) Transfer-Encoding
		EnumTransEnc transferEncoding;
		if (value == "chunked")
			transferEncoding = CHUNK;
		else {
			webserv::logError(ERROR, "BAD_REQUEST", 
				"Transfer Encondig meta data error", 
				"RequestParser::handleFieldValue");
			return false;
		}
		reqMsg.setMetaTransferEncoding(transferEncoding);
	} else if (name == "Content-Type") {//					5) Content-Type
		reqMsg.setMetaContentType(value);
	}
	return true;
}
