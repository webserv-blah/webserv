#include <iostream>

#include "RequestParser.hpp"
#include "utils.hpp"
#include <stdexcept>
#include <sstream>

RequestParser::RequestParser() : oneLineMaxLength_(ONELINE_MAX_LENGTH), uriMaxLength_(URI_MAX_LENGTH) {}
//RequestParser::RequestParser(size_t configBodyLength) {}
RequestParser::~RequestParser() {}

//void RequestParser::setConfigBodyLength(size_t length) {
//	this->bodyMaxLength = length;
//}

std::string RequestParser::parse(const std::string &readData, RequestMessage &reqMsg) {
	std::istringstream iss(readData);
	std::string buffer;
	TypeReqStatus newStatus;

	// 다시 마저 읽어오는 경우도 생각해봐야함
	while (std::getline(iss, buffer, '\n')) {
		//std::cout << "WHILE: " << buffer <<std::endl;
		if (!buffer.empty() && buffer[buffer.size() - 1] == '\r') {
			if (buffer.size() == 1) {
				newStatus = this->setStatusCRLF(reqMsg.getStatus());
				reqMsg.setStatus(newStatus);
				if (newStatus == REQ_HEADER_CRLF)
					break ;// == body
				if (newStatus == REQ_ERROR)
					throw std::logic_error("Error: invalid CLRF location error");
				continue ;
			}
			this->handleOneLine(buffer.erase(buffer.size() - 1), reqMsg);
			if (iss.peek() == EOF)// - [ ] 덜 읽어들인 케이스
				return buffer;// == remain data
		} else {
			throw std::logic_error("Error: single \\n error");
		}
		//////////정상종료시에 connenction & content length 디폴트값설정 해주자!
	}
	iss >> buffer;
	this->parseBody(buffer, reqMsg);//body는 content length만큼 그대로 읽어들임 
	return "";
}


void RequestParser::handleOneLine(const std::string &line, RequestMessage &reqMsg) {
	const TypeReqStatus curStatus = reqMsg.getStatus();
			//request message의 완성도에 따라 
			//각 helper함수를 호출하면 될 것 같다
	if (curStatus == REQ_INIT
	||  curStatus == REQ_TOP_CRLF)
		this->parseStartLine(line, reqMsg);
	if (curStatus == REQ_STARTLINE
	||  curStatus == REQ_HEADER_FIELD)
		this->parseFieldLine(line, reqMsg);
	//if (reqMsg.getStatus() == REQ_HEADER_CRLF) {
	//	this->parseBody(line, reqMsg);//body는 content length만큼 그대로 읽어들임 
	//	//////////////////////////////
	//	break ;//body는 CRLF를 그대로 담아야하기때문에 다르게 처리되어야함
	//}
}

TypeReqStatus RequestParser::setStatusCRLF(const TypeReqStatus &curStatus) {
	// 유효한 것으로 처리되는 두 위치의 CRLF외에는 전부 에러
	if (curStatus == REQ_INIT)
		return REQ_TOP_CRLF;
	if (curStatus == REQ_HEADER_FIELD)
		return REQ_HEADER_CRLF;
	return REQ_ERROR;
	//??? done상태가..???
}

void RequestParser::parseStartLine(const std::string &line, RequestMessage &reqMsg) {
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

	std::vector<std::string> values(1, value);//temp
	reqMsg.addFields(name, values);

	try {
		if (name == "Host")
			reqMsg.setMetaHost(value);
		if (name == "Content-Length")
			reqMsg.setMetaContentLength(value);//int는... 안될 것 같은데..
		if (name == "Connection")
			reqMsg.setMetaConnection(value);
	} catch (const std::exception &e) {
		//it is header field value error
		//how to save this... 
	}

	reqMsg.setStatus(REQ_HEADER_FIELD);
}

void RequestParser::parseBody(const std::string &line, RequestMessage &reqMsg) {
	//if (reqMsg.getMetaContentLength())
	reqMsg.addBody(line);
	//reqMsg.setStatus(REQ_BODY_ING);
	reqMsg.setStatus(REQ_DONE);//우선은 종료 처리... 여기서 마음대로 종료하면 안됨!
}

//void RequestParser::cleanUpChunkedBody() {

//}
