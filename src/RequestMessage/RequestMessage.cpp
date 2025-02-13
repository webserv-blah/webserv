#include "RequestMessage.hpp"
#include <exception>

RequestMessage::RequestMessage() {}
RequestMessage::RequestMessage(TypeMethod method, std::string target) : method_(method), target_(target) {}
RequestMessage::~RequestMessage() {}

void RequestMessage::setMethod(TypeMethod method) {
	this->method_ = method;
}

void RequestMessage::setTarget(std::string target) {
	this->target_ = target;
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
