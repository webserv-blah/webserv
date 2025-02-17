#include "RequestMessage.hpp"
#include <stdexcept>

RequestMessage::RequestMessage() : method_(INIT) {}
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

TypeField RequestMessage::getFields() const {
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

// Parser 구현 후, 파싱 테스트용 함수, C++98 X
#include <iostream>
#include <iterator>
void RequestMessage::printFields(void) const {
	std::cout << "\033[32;7m< PrintFields :\033[0m"<<std::endl;
	for (auto it : this->fieldLines_) {
		std::cout <<"["<<it.first<<"]: ";
		for (auto itt : it.second) {
			std::cout <<"{"<<itt<<"}";
			std::cout <<", ";
		}
		std::cout << std::endl;
	}
	std::cout << "\033[32;7m>\033[0m"<<std::endl;
}
