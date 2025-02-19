#include "EventHandler.hpp"

EventHandler::EventHandler() {

}

EventHandler::~EventHandler() {

}

// 서버 fd에서 발생한 Read 이벤트 처리
int	EventHandler::handleServerReadEvent(int fd) {
    struct sockaddr clientAddr;
    socklen_t addrLen = sizeof(clientAddr);

    int clientFd = accept(fd, (struct sockaddr*)&clientAddr, &addrLen);
    if (clientFd < 0) {
        perror("accept error");
        return -1;
    }

    return clientFd;
}

// 클라이언트 fd에서 발생한 Read 이벤트 처리
int	EventHandler::handleClientReadEvent(ClientSession& clientSession) {
    int status = readRequest(clientSession, parser_);

    if (status == READ_COMPLETE) {
		std::string responseMsg;

		if (cgiHandler_.isCGI(clientSession.getPath())) {
			responseMsg = cgiHandler.handleRequest(clientSession.getReqMsg(), clientSession.getConfig());
		} else {
			responseMsg = staticHandler.handleRequest(clientSession.getReqMsg(), clientSession.getConfig());
		}
		clientSession.setWriteBuffer(responseMsg);
		
        status = sendResponse(clientSession);
    }
    return status;
}

// 클라이언트 fd에서 발생한 Write 이벤트 처리
<<<<<<< HEAD
int EventHandler::handleClientWriteEvent(ClientSession& clientSession) {
    int status = sendResponse(clientSession);
    
    return status;
}

void	EventHandler::handleError(int statusCode, ClientSession& clientSession) {
	clientSession.clearWriteBuffer(); // ClientSession 명세 확인 후 수정 예정

	std::string errorMsg = rspBuilder_.buildError(statusCode);

	sendResponse(clientSession);
}