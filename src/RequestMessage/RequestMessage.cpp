#include "RequestMessage.hpp"
#include <stdexcept>

RequestMessage::RequestMessage() : method_(INIT), status_(REQ_INIT) {}
RequestMessage::RequestMessage(TypeMethod method, std::string targetURI) : method_(method), targetURI_(targetURI) {}
RequestMessage::~RequestMessage() {}

bool RequestMessage::hasMethod() const {
	if (this->method_ == INIT)
		return false;
	return true;
}

RequestMessage::TypeMethod RequestMessage::getMethod() const {
	if (hasMethod())
		return this->method_;
	throw std::exception();//doesn't have a method yet
}

std::string RequestMessage::getTargetURI() const {
	return this->targetURI_;
}

RequestMessage::TypeField RequestMessage::getFields() const {
	return this->fieldLines_;
}

std::string RequestMessage::getBody() const {
	return this->body_;
}

void RequestMessage::setMethod(TypeMethod method) {
	this->method_ = method;
}

void RequestMessage::setTargetURI(std::string targetURI) {
	this->targetURI_ = targetURI;
}

void RequestMessage::addFields(std::string field, std::vector<std::string> values) {
	if (this->fieldLines_.find(field) == this->fieldLines_.end())
		this->fieldLines_[field] = values;
	else
		throw std::exception();//already exist Header Field
}

void RequestMessage::addBody(std::string bodyData) {
	this->body_.append("\r\n");
	this->body_.append(bodyData);
}

RequestMessage::TypeReqStatus RequestMessage::getStatus() const {
	return this->status_;
}

std:: string RequestMessage::getMetaHost() const {
	return this->metaHost_;
}

ssize_t RequestMessage::getMetaConnection() const {
	return this->metaConnection_;
}

ssize_t RequestMessage::getMetaContentLength() const {
	return this->metaContentLength_;
}

void RequestMessage::setStatus(TypeReqStatus status) {
	this->status_ = status;
}

void RequestMessage::setMetaHost(std::string value) {
	this->metaHost_ = value;
}

void RequestMessage::setMetaConnection(ssize_t value) {
	this->metaConnection_ = value;
}

void RequestMessage::setMetaContentLength(ssize_t value) {
	this->metaContentLength_ = value;
}



// Parser 구현 후, 파싱 테스트용 함수, C++98 X
#include <iostream>
#include <iterator>
void RequestMessage::printResult() const {
	std::cout << "\033[32;7m PrintStartLine :\033[0m"<<std::endl;

	std::cout <<"\033[37;2mmethod: \033[0m";
	if (this->method_ == GET)
		std::cout <<"GET;"<<std::endl;
	else if (this->method_ == POST)
		std::cout <<"POST;"<<std::endl;
	else if (this->method_ == DELETE)
		std::cout <<"DELETE;"<<std::endl;
	else if (this->method_ == INIT)
		std::cout <<"INIT;"<<std::endl;
	else
		std::cout <<"(none);"<<std::endl;
	std::cout <<"\033[37;2muri: \033[0m";
	std::cout <<this->targetURI_<<";"<<std::endl;
	this->printFields();
	this->printBody();
}
void RequestMessage::printFields(void) const {
	std::cout << "\033[32;7m PrintFields :\033[0m"<<std::endl;
	for (auto it : this->fieldLines_) {
		int cnt = 0;
		std::cout <<it.first<<": {";
		for (auto itt : it.second) {
			if (cnt != 0)
			std::cout << ", ";
			std::cout << itt;
			cnt++;
		}
		std::cout <<"};"<<std::endl;
	}
}
void RequestMessage::printBody(void) const {
	std::cout << "\033[32;7m PrintBody :\033[0m"<<std::endl;
	for (auto it = this->body_.begin(); it != this->body_.end(); ++it) {
        if (*it == '\n')
            std::cout << "\\n" << std::endl;
        else if (*it == '\r')
            std::cout << "\\r";
        else
            std::cout << *it;
	}
}
