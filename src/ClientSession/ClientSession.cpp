#include "ClientSession.hpp"

#ifndef BUFFER_SIZE
#define BUFFER_SIZE 8192
#endif

ClientSession::ClientSession(int listenFd, int clientFd, std::string clientIP) : listenFd_(listenFd), clientFd_(clientFd), reqMsg_(NULL), config_(NULL), clientIP_(clientIP) {
	this->readBuffer_.reserve(BUFFER_SIZE * 2);
}
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

int ClientSession::getErrorStatusCode() const {
	return this->errorStatusCode_;
}

const RequestMessage *ClientSession::getReqMsg() const {
	return this->reqMsg_;
}

const RequestConfig *ClientSession::getConfig() const {
	return this->config_;
}

std::string ClientSession::getReadBuffer() const {
	return this->readBuffer_;
}

std::string ClientSession::getWriteBuffer() const {
	return this->writeBuffer_;
}

std::string	ClientSession::getClientIP() const {
	return this->clientIP_;
}

void ClientSession::setListenFd(const int &listenFd) {
	this->listenFd_ = listenFd;
}

void ClientSession::setClientFd(const int &clientFd) {
	this->clientFd_ = clientFd;
}

void ClientSession::setErrorStatusCode(const int &statusCode) {
	this->errorStatusCode_ = statusCode;
}

void ClientSession::setReqMsg(RequestMessage *reqMsg) {
	this->reqMsg_ = reqMsg;
}

void ClientSession::setConfig(const RequestConfig *config) {
	this->config_ = config;
}

void ClientSession::setReadBuffer(const std::string &remainData) {
	this->readBuffer_ = remainData;
}

void ClientSession::setWriteBuffer(const std::string &remainData) {
	this->writeBuffer_ = remainData;
}

RequestMessage &ClientSession::accessReqMsg() {
	return *this->reqMsg_;
}

std::string &ClientSession::accessReadBuffer() {
	return this->readBuffer_;
}

void ClientSession::resetRequest() {
	delete this->reqMsg_;
	this->reqMsg_ = NULL;
	this->config_ = NULL;
}
