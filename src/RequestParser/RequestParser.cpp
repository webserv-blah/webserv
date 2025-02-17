#include <iostream>

#include "RequestParser.hpp"
#include "utils.hpp"
#include <stdexcept>
#include <sstream>

RequestParser::RequestParser() : oneLineMaxLength_(ONELINE_MAX_LENGTH), uriMaxLength_(URI_MAX_LENGTH) {}
//RequestParser::RequestParser(size_t configBodyLength) {}
RequestParser::~RequestParser() {}

std::string RequestParser::parse(const std::string &readData, RequestMessage &reqMsg) {
	std::istringstream iss(readData);
	std::string buffer;

	//if (reqMsg.getStatus() == REQ_BODY)
		//body는 content length만큼 그대로 읽어들임 
	while (std::getline(iss, buffer, '\n')) {
		//TypeReqStatus curStatus = reqMsg.getStatus();//왜 타입 안되는거지
		RequestMessage::TypeReqStatus curStatus = reqMsg.getStatus();
		std::cout << "current status:"<<curStatus<<"\n";
		if (!buffer.empty() && buffer[buffer.size() - 1] == '\r') {
			if (buffer.size() == 1) {
				reqMsg.setStatus(this->setStatusCRLF(curStatus));
				curStatus = reqMsg.getStatus();
			}

			//request message의 완성도에 따라 
			//각 helper함수를 호출하면 될 것 같다
			buffer.erase(buffer.size() - 1);
			if (curStatus == REQ_INIT
			||  curStatus == REQ_TOP_CRLF)
				this->parseStartLine(buffer, reqMsg);
			if (curStatus == REQ_STARTLINE
			||  curStatus == REQ_HEADER_FIELD_ING)
				this->parseFieldLine(buffer, reqMsg);
			if (curStatus == REQ_HEADER_CRLF)
				this->parseBody(buffer, reqMsg);
			if (iss.peek() == EOF)// - [ ] 덜 읽어들인 케이스
				return buffer;// == remain data
			
		} else {
			throw std::logic_error("Error: single \\n error");
		}
		//////////정상종료시에 connenction & content length 디폴트값설정 해주자!
	}
	return NULL;
}

//void RequestParser::setConfigBodyLength(size_t length) {
//	this->bodyMaxLength = length;
//}

RequestMessage::TypeReqStatus RequestParser::setStatusCRLF(const RequestMessage::TypeReqStatus &curStatus) {
	if (curStatus == REQ_INIT)
		return REQ_TOP_CRLF;
	if (curStatus == REQ_STARTLINE)
		return REQ_STARTLINE_CRLF;
	if (curStatus == REQ_HEADER_FIELD_ING)
		return REQ_HEADER_CRLF;
	//??? done상태가..???
	return REQ_ERROR;
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

	if (name == "Content-Length")
		reqMsg.setMetaContentLength(utils::stoi(value));//int는... 안될 것 같은데..

	reqMsg.setStatus(REQ_HEADER_FIELD_ING);
}

void RequestParser::parseBody(const std::string &line, RequestMessage &reqMsg) {
	//if (reqMsg.getMetaContentLength())
	reqMsg.addBody(line);
	std::cout <<  "BODYBIDY:" << line << std::endl; 
	//reqMsg.setStatus(REQ_BODY_ING);
	reqMsg.setStatus(REQ_DONE);
}

//void RequestParser::cleanUpChunkedBody() {

//}
