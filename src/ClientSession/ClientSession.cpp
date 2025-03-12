#include "ClientSession.hpp"

#ifndef BUFFER_SIZE
#define BUFFER_SIZE 8192
#endif

ClientSession::ClientSession(int listenFd, int clientFd, std::string clientIP) : listenFd_(listenFd), clientFd_(clientFd), clientIP_(clientIP), reqMsg_(NULL), config_(NULL) {
	this->readBuffer_.reserve(BUFFER_SIZE * 2);
	this->defConfig_ = GlobalConfig::getInstance().findRequestConfig(listenFd, "", "");
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
	if (config_) {
		return this->config_;
	}
	return this->defConfig_;
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

const CgiProcessInfo *ClientSession::getCgiProcessInfo() const {
	return &(this->cgiProcessInfo_);
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
	resetRequest();
}

void ClientSession::setCgiProcessInfo(CgiProcessInfo &cgiProcessInfo) {
	this->cgiProcessInfo_ = cgiProcessInfo;
}

bool ClientSession::isReceiving() const {
	if (this->reqMsg_) {
		return true;
	}
	return false;
}

RequestMessage &ClientSession::accessReqMsg() {
	return *this->reqMsg_;
}

std::string &ClientSession::accessReadBuffer() {
	return this->readBuffer_;
}

CgiProcessInfo	&ClientSession::accessCgiProcessInfo() {
	return this->cgiProcessInfo_;
}

void ClientSession::resetRequest() {
	delete this->reqMsg_;
	this->reqMsg_ = NULL;
	this->config_ = NULL;
}
