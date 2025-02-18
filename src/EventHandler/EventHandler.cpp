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

    if (status == TypeSesStatus::READ_COMPLETE) {
		//cgi or static Handler 호출

		//if (static) build response

		//send response
        status = sendResponse(clientSession);
    }
    return status;
}

// 클라이언트 fd에서 발생한 Write 이벤트 처리
int	EventHandler::handleClientWriteEvent(ClientSession& clientSession) {
    int status = sendResponse(clientSession);
    return status;
}

// 예외 이벤트 처리
void    EventHandler::handleExceptionEvent(ClientSession& clientSession) {
    // Q.Exception Event 발생 케이스 및 처리 내용 구체화
}

// timeout 처리(408반환)
void	EventHandler::handleTimeout(ClientSession& clientSession) {
    // Q.error 핸들링 시에도 status 반환 필요할지
    sendErrorResponse(clientSession, 408);
}

// 서버 종료 전 연결되어 있는 client에 연결 종료 고지
void	EventHandler::handleServerShutDown(ClientSession& clientSession) {
    sendErrorResponse(clientSession, 503);
}



