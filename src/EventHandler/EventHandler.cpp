#include "EventHandler.hpp"

EventHandler::EventHandler() : staticHandler_(rspBuilder_) {

}

EventHandler::~EventHandler() {

}

// 서버 fd에서 발생한 Read 이벤트 처리
int	EventHandler::handleServerReadEvent(int fd, ClientManager& clientManager) {
    struct sockaddr clientAddr;
    socklen_t addrLen = sizeof(clientAddr);

    int clientFd = accept(fd, (struct sockaddr*)&clientAddr, &addrLen);
    if (clientFd < 0) {
        perror("client accept error");
    }

    clientManager.addClient(fd, clientFd, /*string으로 변환된 clientAddr*/);

    return clientFd;
}

// 클라이언트 fd에서 발생한 Read 이벤트 처리
int	EventHandler::handleClientReadEvent(ClientSession& clientSession) {
    int status = recvRequest(clientSession);
    //recvRequest: EventHandler의 멤버 함수
    //parser_: EventHandler의 멤버 변수

    if (status == READ_COMPLETE) {
        RequestMessage  requestMsg = clientSession.getReqMsg();
		std::string     responseMsg;

        // CGI 실행 여부 판별 및 응답 생성
		if (cgiHandler_.isCGI(requestMsg.getTargetURI())) {
			responseMsg = cgiHandler_.handleRequest(clientSession);
		} else {
			responseMsg = staticHandler_.handleRequest(requestMsg, clientSession.getConfig());
		}
        // 생성된 응답을 clientSession 내 write 버퍼에 저장
		clientSession.setWriteBuffer(responseMsg);
		
        // 응답 전송 시도 및 sessionStatus 갱신
        status = sendResponse(clientSession);
    } else if (status == REQUEST_ERROR) {
        int statusCode = clientSession.getErrorStatusCode();

        handleError(statusCode, clientSession);
    }

    return status;
}

// 클라이언트 fd에서 발생한 Write 이벤트 처리
int EventHandler::handleClientWriteEvent(ClientSession& clientSession) {
    // 응답 전송 시도 및 sessionStatus 갱신
    int status = sendResponse(clientSession);
    
    return status;
}

void	EventHandler::handleError(int statusCode, ClientSession& clientSession) {
	// 에러 응답 생성
    std::string errorMsg = rspBuilder_.buildError(statusCode, clientSession.getConfig());

    // 생성된 응답을 clientSession 내 write 버퍼에 저장
	clientSession.setWriteBuffer(errorMsg);

    // 응답 전송
	sendResponse(clientSession);
}