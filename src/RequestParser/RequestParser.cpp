#include <iostream>//중간 과정을 출력하기 위한 임시 헤더

#include "RequestParser.hpp"
#include "utils.hpp"
#include <stdexcept>
#include <sstream>

RequestParser::RequestParser() : oneLineMaxLength_(ONELINE_MAX_LENGTH), uriMaxLength_(URI_MAX_LENGTH) {}
RequestParser::~RequestParser() {}

//void RequestParser::setConfigBodyLength(size_t length) {
	// MUST TO DO: Config를 반영하는 로직 필요, Body파싱 구현시 하게 될 듯
//	this->bodyMaxLength = length;
//}

std::string RequestParser::parse(const std::string &readData, RequestMessage &reqMsg) {
	std::istringstream iss(readData);
	std::string buffer;
	EnumReqStatus status = reqMsg.getStatus();
	
	//std::cout << "SPLIT: " << readData <<";"<<std::endl;
	//std::cout << "STATUS: " << status <<";"<<std::endl;
	if (status != REQ_HEADER_CRLF
	&&  status != REQ_BODY) {
		while (std::getline(iss, buffer, '\n')) {
			//std::cout << "BUFFER: " << buffer.substr(0, buffer.size()-1) <<";"<<std::endl;
			if (iss.eof())// \n이 나오지 않고 readData가 끝난 상태. 다음 loop로 넘어감 
				return buffer;
			if (!buffer.empty() && buffer[buffer.size() - 1] == '\r') {
				if (buffer.size() == 1) {
					status = this->handleCRLFLine(reqMsg.getStatus());
					reqMsg.setStatus(status);
					if (status == REQ_HEADER_CRLF) {
						// MUST TO DO: meta data 기본값 설정
						std:getline(iss, buffer, '\0');
						break ;// == body
					}
					if (status == REQ_ERROR)
						throw std::logic_error("Error: invalid CLRF location error");
				} else {
					this->handleOneLine(buffer.erase(buffer.size() - 1), reqMsg);
				}
			} else {

				throw std::logic_error("Error: single \\n error");
			}
		}
	} else {
		buffer = readData;
	}

	if (buffer.length() == 0)
		return "";
	if (reqMsg.getMetaTransferEncoding() == CHUNK)
		return this->cleanUpChunkedBody(buffer, reqMsg);
	else
		return this->parseBody(buffer, reqMsg);
}


void RequestParser::handleOneLine(const std::string &line, RequestMessage &reqMsg) {
	const EnumReqStatus curStatus = reqMsg.getStatus();

	if (curStatus == REQ_INIT
	||  curStatus == REQ_TOP_CRLF)
		this->parseStartLine(line, reqMsg);
	if (curStatus == REQ_STARTLINE
	||  curStatus == REQ_HEADER_FIELD)
		this->parseFieldLine(line, reqMsg);
	//if (reqMsg.getStatus() == REQ_HEADER_CRLF) {
	//	this->parseBody(line, reqMsg);//body는 content length만큼 그대로 읽어들임 
	//	break ;//body는 CRLF를 그대로 담아야하기때문에 다르게 처리되어야함
	//}
}

//현재 RequestMessage 상태에서 CRLF줄이 유효한지 검증하고, 다음 상태를 지정해주는 함수
EnumReqStatus RequestParser::handleCRLFLine(const EnumReqStatus &curStatus) {
	// 유효한 것으로 처리되는 두 위치의 CRLF외에는 전부 에러
	if (curStatus == REQ_INIT)
		return REQ_TOP_CRLF;
	if (curStatus == REQ_HEADER_FIELD)
		return REQ_HEADER_CRLF;// MUST TO DO: 이 상태에서 request가 새롭게 들어오면 DONE을 해줘야하는 상황이 있음
	return REQ_ERROR;
	// MUST TO DO: 이상태에서 DONE의 판단을 할 수 있는가?
}

void RequestParser::parseStartLine(const std::string &line, RequestMessage &reqMsg) {
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
			throw std::logic_error("not allow method");
		}
	}

	if (std::getline(iss, buffer, ' '))
		reqMsg.setTargetURI(buffer);
	else
		throw std::logic_error("none target uri");

	iss >> buffer;
	if (buffer == "HTTP/1.1")
		reqMsg.setStatus(REQ_STARTLINE);
	else {
		reqMsg.setStatus(REQ_ERROR);
		throw std::logic_error("http version error");
	}
}

void RequestParser::parseFieldLine(const std::string &line, RequestMessage &reqMsg) {
	std::istringstream iss(line);
	std::string name;
	std::string value;

	std::getline(iss, name, ':');
	iss.get();//trim 임시
	iss >> value;

	std::vector<std::string> values(1, value);// MUST TO DO: 여러 value를 처리하는 로직
	reqMsg.addFields(name, values);

	try {
		handleFieldValue(name, value, reqMsg);
	} catch (const std::exception &e) {
		std::cerr << e.what() << "\033[37;2m//in Fieldline()\033[0m" << std::endl;
	}

	reqMsg.setStatus(REQ_HEADER_FIELD);
}

void RequestParser::handleFieldValue(const std::string &name, const std::string &value, RequestMessage &reqMsg) {
	if (name == "Host") {
		reqMsg.setMetaHost(value);
	} else if (name == "Connection") {
		EnumConnect connection;
		if (value == "keep-alive")
			connection = KEEP_ALIVE;
		else if (value == "close")
			connection = CLOSE;
		else
			throw std::logic_error("invalid field value: Connection");
		reqMsg.setMetaConnection(connection);
	} else if (name == "Content-Length") {
		size_t contentLength = utils::sto_size_t(value);
		reqMsg.setMetaContentLength(contentLength);
	} else if (name == "Transfer-Encoding") {
		EnumTransEnc transferEncoding;
		if (value == "chunked")
			transferEncoding = CHUNK;
		else
			throw std::logic_error("invalid field value: Connection");
		reqMsg.setMetaTransferEncoding(transferEncoding);
	}
}

// Content-Length의 길이에 따라 예외 처리됨. 유효한 문자열은 기존의 Body의 추가함
std::string RequestParser::parseBody(const std::string &line, RequestMessage &reqMsg) {
	const size_t contentLength = reqMsg.getMetaContentLength();

	if (contentLength >= reqMsg.getBodyLength() + line.length()) {
		reqMsg.addBody(line);
		if (contentLength == reqMsg.getBodyLength())
			reqMsg.setStatus(REQ_DONE);
		else
			reqMsg.setStatus(REQ_BODY);
		return "";
	}

	size_t substrSize = contentLength - reqMsg.getBodyLength();
	reqMsg.addBody(line.substr(0, substrSize));
	std::string remainData = line.substr(substrSize);
	
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
		throw std::logic_error("body -> suspicious request");
	}
	reqMsg.setStatus(REQ_DONE);
	return remainData;
}

std::string RequestParser::cleanUpChunkedBody(const std::string &data, RequestMessage &reqMsg) {
	std::istringstream iss(data);
	std::string buffer;
	std::string tmp;
	size_t chunkSize;

	while (std::getline(iss, buffer, '\n')) {
		if (iss.eof())
			return buffer;
		if (!buffer.empty() && buffer[buffer.size() - 1] == '\r') {
			// iss에 남은 길이 측정
			std::streampos currentPos = iss.tellg();
			iss.seekg(0, std::ios::end);
			std::streampos remainISS = iss.tellg() - currentPos;
			iss.seekg(currentPos);

			chunkSize = utils::sto_size_t(buffer.erase(buffer.size() - 1));
			//std::cout << "CHUNKSIZE: " << chunkSize<<";"<<std::endl;
			if (chunkSize == 0) {
				reqMsg.setStatus(REQ_DONE);
				reqMsg.setMetaContentLength(reqMsg.getBodyLength());
				return iss.str().substr(static_cast<std::string::size_type>(currentPos)); 
			}
			
			tmp = buffer + "\r\n";
			
			if (remainISS < chunkSize + 2)
				return tmp + iss.str().substr(static_cast<std::string::size_type>(currentPos));

			buffer.resize(chunkSize + 2);
			iss.read(&buffer[0], chunkSize + 2);

			if (buffer.compare(buffer.size() - 2, 2, "\r\n") != 0)
				throw std::logic_error("chunk format error: chunk data");
			reqMsg.addBody(buffer.substr(0, buffer.size() - 2));
		} else {
			throw std::logic_error("chunk format error: chunk size");
		}
	}
}
