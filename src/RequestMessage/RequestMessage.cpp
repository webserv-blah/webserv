#include "RequestMessage.hpp"
#include "utils.hpp"
#include <stdexcept>

RequestMessage::RequestMessage() : method_(INIT), status_(REQ_INIT) {}
RequestMessage::RequestMessage(TypeMethod method, std::string targetURI) : method_(method), targetURI_(targetURI) {}
RequestMessage::~RequestMessage() {}

bool RequestMessage::hasMethod() const {
	if (this->method_ == INIT)
		return false;
	return true;
}

TypeMethod RequestMessage::getMethod() const {
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
	this->body_.append(bodyData);
	this->body_.append("\r\n");
}

TypeReqStatus RequestMessage::getStatus() const {
	return this->status_;
}

std:: string RequestMessage::getMetaHost() const {
	return this->metaHost_;
}

TypeConnection RequestMessage::getMetaConnection() const {
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

void RequestMessage::setMetaConnection(std::string value) {
	if (value == "keep-alive")
		this->metaConnection_ = KEEP_ALIVE;
	if (value == "close")
		this->metaConnection_ = CLOSE;
	throw std::logic_error("Error: connection value");
}

void RequestMessage::setMetaContentLength(std::string value) {
	this->metaContentLength_ = utils::stosizet(value);
}

// Parser 구현 후, 파싱 테스트용 함수, C++98 X
#include <iostream>
#include <iterator>
void RequestMessage::printResult() const {
	std::cout << "\033[32;7m StartLine :\033[0m"<<std::endl;
	std::cout <<"\033[37;2mmethod: \033[0m";
	const char *method;
	switch (this->method_) {
		case INIT:
			method = "INIT";
		case GET:
			method = "GET";
		case POST:
			method = "POST";
		case DELETE:
			method = "DELETE";
	}
	std::cout <<method<<";"<<std::endl;
	
	std::cout <<"\033[37;2muri: \033[0m";
	std::cout <<this->targetURI_<<";"<<std::endl;
	this->printFields();
	this->printBody();
	this->printMetaData();
}
void RequestMessage::printFields(void) const {
	std::cout << "\033[32;7m Fields :\033[0m"<<std::endl;
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
	std::cout << "\033[32;7m Body :\033[0m"<<std::endl;
	std::cout <<"\033[37;2mcount: "<<this->body_.length()<<"\033[0m\n";
	for (auto it = this->body_.begin(); it != this->body_.end(); ++it) {
        if (*it == '\n')
            std::cout << "\\n" << std::endl;
        else if (*it == '\r')
            std::cout << "\\r";
        else
            std::cout << *it;
	}
}
void RequestMessage::printMetaData(void) const {
	std::cout << "\033[37;7m MetaData :\033[0m"<<std::endl;
	std::cout <<"\033[37;2mHost_: "<<this->metaHost_ <<";\033[0m\n";
	std::cout <<"\033[37;2mConnection_: ";
	const char *connection;
	switch (this->metaConnection_) {
		case KEEP_ALIVE:
			connection = "keep-alive";
		case CLOSE:
			connection = "close";
	}
	std::cout<<connection <<";\033[0m\n";
	std::cout <<"\033[37;2mContentLength_: "<<this->metaContentLength_ <<";\033[0m\n";
}