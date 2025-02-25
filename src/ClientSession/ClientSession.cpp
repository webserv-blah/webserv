#include "ClientSession.hpp"

ClientSession::ClientSession(int listenFd, int clientFd, std::string clientAddr) : status_(READ_CONTINUE), listenFd_(listenFd), clientFd_(clientFd), clientAddr_(clientAddr), reqMsg_(NULL), config_(NULL) {}

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

std::string	ClientSession::getClientAddr() const {
	return this->clientAddr_;
}

std::string ClientSession::getReadBuffer() const {
	return this->readBuffer_;
}

std::string ClientSession::getWriteBuffer() const {
	return this->writeBuffer_;
}

const RequestMessage &ClientSession::getReqMsg() const {
	return *this->reqMsg_;
}

const RequestConfig &ClientSession::getConfig() const {
	return *this->config_;
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

// EventHandler::recvRequest에서 호출되어 요청데이터를 파싱하고 결과를 판단하는 함수
// parser: EventHandler가 들고있는 주요로직객체중 하나
// readData: recv()함수에서 새로 읽어들인 요청 데이터
EnumSesStatus ClientSession::implementReqMsg(RequestParser &parser, const std::string &readData) {
	// 새로운 요청일 때, RequestMessage 동적할당
	if (this->reqMsg_ == NULL)
		this->reqMsg_ = new RequestMessage();// 이후 요청처리(handler&builder) 완료 후, delete 필요

	// 해당 ClientSession의 RequestMessage를 파싱하기 전에 Body의 최대 길이를 설정
	if (this->config_ != NULL)
		parser.setConfigBodyLength(this->config_->clientMaxBodySize_.value());
	else
		parser.setConfigBodyLength(BODY_MAX_LENGTH);

	// 이전에 버퍼 저장해놓은 데이터와 새로운 요청 데이터를 가지고 파싱
	this->errorStatusCode_ = parser.parse(readData, this->readBuffer_, *this->reqMsg_);

	// field-line까지 다 읽은 후, 요청메시지에 맞는 RequestConfig를 설정하고 Host헤더필드 유무를 검증함
	if (this->reqMsg_->getStatus() == REQ_HEADER_CRLF) {
		const GlobalConfig &globalConfig = GlobalConfig::getInstance();
		this->config_ = globalConfig.findRequestConfig(this->listenFd_, this->reqMsg_->getMetaHost(), this->reqMsg_->getTargetURI());
		if (this->reqMsg_->getMetaHost().empty()) {
			this->errorStatusCode_ = BAD_REQUEST;
			return REQUEST_ERROR;
		}
		if (this->reqMsg_->getMetaContentLength() == 0
		&&  this->reqMsg_->getMetaTransferEncoding() == NONE_ENCODING) {
			this->reqMsg_->setStatus(REQ_DONE);
			return READ_COMPLETE;
		}
	}

	// 파싱이 끝나고 나서, 에러 status code와 RequestMessage의 상태를 점검함
	if (this->errorStatusCode_ != NONE_STATUS_CODE)
		return REQUEST_ERROR;
	else if (this->reqMsg_->getStatus() == REQ_DONE)
		return READ_COMPLETE;
	return READ_CONTINUE;
}
