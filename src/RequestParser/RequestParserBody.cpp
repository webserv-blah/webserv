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
	
	while () {
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
		}
	}

	



	std::istringstream iss(readBuffer);
	std::string buffer;
	std::string tmp;
	size_t chunkSize;

	while (std::getline(iss, buffer, '\n')) {
		if (iss.eof()) {// 5-1. \n으로 getline되지 않은 경우 readBuffer에 남겨 다음 loop에 진행 
			std::cout << "CHUNK eof RETURN;\n";
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
			
			// 읽어낸 데이터가 모자를 경우 readBuffer에 돌려놓기 위함
			tmp = buffer + "\r\n";
			// 남은 데이터가 청크사이즈보다 모자를때 파싱을 미룸
			// chunkSize가 0일때도 "0\r\n\r\n"이 정확한 종료기준이기 때문에 종료확정을 미룸
			if (static_cast<size_t>(endPos - currentPos) < chunkSize + 2) {
				readBuffer = tmp + iss.str().substr(static_cast<std::string::size_type>(currentPos));
				std::cout << "모잘라 RETURN;\n";
				return NONE_STATUS_CODE;
			}
			
			// chunkSize가 0일때 Body파싱을 종료함
			if (chunkSize == 0) {
				std::getline(iss, buffer, '\0');
				if (buffer.find(CRLF, 0) != 0)
					return BAD_REQUEST;
				reqMsg.setStatus(REQ_DONE);
				reqMsg.setMetaContentLength(reqMsg.getBodyLength());
				readBuffer = buffer.erase(0, 2);
				std::cout << "청크 0 끝내!! RETURN;\n";
				return NONE_STATUS_CODE;
			}

			// 파싱한 청크사이즈 기준으로 청크 데이터를 읽음
			buffer.resize(chunkSize + 2);
			iss.read(&buffer[0], chunkSize + 2);

			// 청크 데이터가 \r\n형식에 맞춰 들어왔는지 검증
			if (buffer.compare(buffer.size() - 2, 2, "\r\n") != 0) {
				webserv::logError(ERROR, "BAD_REQUEST", 
					"invalid chunk data format", 
					"RequestParser::cleanUpChunkedBody");
				return BAD_REQUEST;//status code: 유효하지 않은 청크 데이터 형식
			}
				
			if (this->bodyMaxLength_ < reqMsg.getBodyLength() + chunkSize) {
				webserv::logError(ERROR, "CONTENT_TOO_LARGE", 
					"Body size is larger than the Config", 
					"RequestParser::cleanUpChunkedBody");
				return CONTENT_TOO_LARGE;//status code: Body가 설정보다 큼 “Request Entity Too Large”
			}

				// 청크 데이터에 \r\n를 제거하여 Body에 저장
			reqMsg.addBody(buffer.substr(0, buffer.size() - 2));
		} else { //\r\n으로 이루어져있지 않음
			webserv::logError(ERROR, "BAD_REQUEST", 
				"single LF", 
				"RequestParser::cleanUpChunkedBody");
			return BAD_REQUEST;//status code: CRLF가 아닌, 단일 LF
		}
	}
	readBuffer = "";
	return NONE_STATUS_CODE;
}
