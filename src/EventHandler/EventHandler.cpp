#include "EventHandler.hpp"

EventHandler::EventHandler() {
    // Q.필요한 멤버 변수 및 초기화 여부 + 초기화 시 필요한 정보
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

    // Q.여기서 ClientSession 생성? + 생성시 필요한 정보 목록
    // Q.IPv4와 IPv6에 따라 분기처리가 필요한 부분이 있는지

    return clientFd;
}


// Q.recv, send status에 대한 매크로 혹은 enum 논의 필요
// ServerManager, EventHandler에서 사용 => 공통 헤더 이용?

// 클라이언트 fd에서 발생한 Read 이벤트 처리
int	EventHandler::handleClientReadEvent(int fd) {
    int status = readRequest();

    if (status == DONE_READING) {
        status = sendReponse();
    }
    return status;
}

// 클라이언트 fd에서 발생한 Write 이벤트 처리
int EventHandler::handleClientWriteEvent(int fd) {
    int status = sendResponse();
    
    return status;
}

// 예외 이벤트 처리
void    EventHandler::handleExceptionEvent(int fd) {
    // Q.Exception Event 발생 케이스 및 처리 내용 구체화
}

// timeout 처리(408반환)
void	EventHandler::handleTimeout(int fd) {
    // Q.error 핸들링 시에도 status 반환 필요할지
    sendResponse(fd, 408);
}

// 서버 종료 전 연결되어 있는 client에 연결 종료 고지
void	EventHandler::handleServerShutDown(int fd) {
    sendResponse(fd, 503);
}



