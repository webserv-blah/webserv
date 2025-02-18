#include "ClientSession.hpp"

ClientSession::ClientSession(int listenFd, int clientFd) : status_(READ_CONTINUE), listenFd_(listenFd), clientFd_(clientFd), reqMsg_(NULL), config_(NULL) {}
ClientSession::~ClientSession() {
	if (this->reqMsg_ != NULL)
		delete this->reqMsg_;
}

int ClientSession::getListenFd() const {
	return this->listenFd_;
}

int ClientSession::getClientFd() const {
	return this->clientFd_;
}

EnumSesStatus ClientSession::getStatus() const {
	return this->status_;
}

std::string ClientSession::getReadBuffer() const {
	return this->readBuffer_;
}

std::string ClientSession::getWriteBuffer() const {
	return this->writeBuffer_;
}

void ClientSession::setListenFd(const int &listenFd) {
	this->listenFd_ = listenFd;
}

void ClientSession::setClientFd(const int &clientFd) {
	this->clientFd_ = clientFd;
}

void ClientSession::setStatus(const EnumSesStatus &status) {
	this->status_ = status;
}

void ClientSession::setReadBuffer(const std::string &remainData) {
	this->readBuffer_ = remainData;
}

void ClientSession::setWriteBuffer(const std::string &remainData) {
	this->writeBuffer_ = remainData;
}

//Optional<ssize_t> ClientSession::getConfBodyMax() const {
//	if (this->config_ == NULL)
//		return Optional<ssize_t>();
//	return this->config_->clientMaxBodySize_;//size_t & ssize_t는 맞춰봐야함
//}

#include <iostream>
EnumSesStatus ClientSession::implementReqMsg(RequestParser &parser, const std::string &readData) {
	if (this->reqMsg_ == NULL)
		this->reqMsg_ = new RequestMessage();
	
	try {
		//parser.setBodyMaxLength(this->config_);
		this->readBuffer_ = parser.parse(this->readBuffer_ + readData, *this->reqMsg_);
		//if (this->reqMsg_->getStatus() != DONE)
		//	this->status_ = ;
		//	?? throw?
		this->reqMsg_->printResult();
	} catch (const std::exception &e) {
		std::cerr << e.what() << "\033[37;2m//in ClientSeesion.implementReqMsg()\033[0m" << std::endl;
		this->status_ = CONNECTION_ERROR;
	}
	return this->status_;
}
