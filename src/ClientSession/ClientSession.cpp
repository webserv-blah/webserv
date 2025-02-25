#include "ClientSession.hpp"

ClientSession::ClientSession(int listenFd, int clientFd, std::string clientIP) : listenFd_(listenFd), clientFd_(clientFd), status_(READ_CONTINUE), reqMsg_(NULL), config_(NULL), clientIP_(clientIP) {}
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

EnumSesStatus ClientSession::getStatus() const {
	return this->status_;
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

size_t ClientSession::getReadCursor() const {
	return this->readCursor_;
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

void ClientSession::setStatus(const EnumSesStatus &status) {
	this->status_ = status;
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

void ClientSession::setReadCursor(const size_t &curCursor) {
	this->readCursor_ = curCursor;
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
