#include "EventHandler.hpp"
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "errorUtils.hpp"

// ResponseBuilder를 인자로 하여 정적 핸들러와 CGI 핸들러를 초기화합니다.
EventHandler::EventHandler() : staticHandler_(responseBuilder_), cgiHandler_(responseBuilder_) {

}

EventHandler::~EventHandler() {

}

//---------------------------------------------------------------------
// 서버 소켓(fd)에서 발생한 읽기 이벤트 처리
// - 클라이언트 연결 요청을 수락하고, 클라이언트의 파일 디스크립터를 반환합니다.
// - 연결 수락에 실패하면 -1을 반환합니다.
//---------------------------------------------------------------------
int EventHandler::handleServerReadEvent(int fd, ClientManager& clientManager) {
    
    struct sockaddr_in clientAddr;              // 클라이언트 주소 정보(IPv4)
    socklen_t addrLen = sizeof(clientAddr);     // 클라이언트 주소 구조체의 크기

    // accept() 호출을 통해 클라이언트 연결 수락
    int clientFd = accept(fd, reinterpret_cast<struct sockaddr*>(&clientAddr), &addrLen);
    if (clientFd < 0) {
        // 연결 수락 실패 - 시스템 호출 에러를 로깅
        webserv::logSystemError(ERROR, "accept", 
                              "Server fd: " + utils::size_t_tos(fd), 
                              "EventHandler::handleServerReadEvent");
        return -1;
    }

    // 클라이언트의 IP 주소 문자열로 변환
    std::string clientIP(inet_ntoa(clientAddr.sin_addr));

    // ClientManager에 새 클라이언트 정보를 추가
    clientManager.addClient(fd, clientFd, clientIP);
	DEBUG_LOG("[EventHandler]New client connected: " + clientIP + ", clientFd: " + utils::int_tos(clientFd))


    // 수락된 클라이언트의 파일 디스크립터 반환
    return clientFd;
}

//---------------------------------------------------------------------
// 클라이언트 소켓(fd)에서 발생한 읽기 이벤트 처리
// - 클라이언트로부터 요청 데이터를 수신하고, 요청 처리를 진행합니다.
// - 요청 처리가 정상적으로 완료되면 응답 전송을 시도합니다.
// - 요청 처리 중 에러가 발생하면 에러 응답을 전송합니다.
//---------------------------------------------------------------------
EnumSesStatus EventHandler::handleClientReadEvent(ClientSession& clientSession) {
    // 클라이언트의 요청 데이터를 수신
    EnumSesStatus status = recvRequest(clientSession);

    if (status == READ_COMPLETE) {
        // 요청 데이터 수신이 완료된 경우
        const RequestMessage&   requestMsg = *clientSession.getReqMsg();
		const RequestConfig&	reqConfig = *clientSession.getConfig();
        std::string     responseMsg;

		#ifdef DEBUG
		std::cout << "[EventHandler]Request Config" << std::endl;
		reqConfig.print(0);
		std::cout << std::endl;
		std::cout << "[EventHandler]Request Message" << std::endl;
		requestMsg.printResult();
		std::cout << std::endl;
		#endif
		if (reqConfig.returnStatus_) {
			// 리다이렉션
			DEBUG_LOG("[EventHandler]Redirection requested")
			responseMsg = handleRedirection(reqConfig);
		} else if (cgiHandler_.isCGI(requestMsg.getMethod(), requestMsg.getTargetURI(), reqConfig)) {
            // CGI 요청
			DEBUG_LOG("[EventHandler]CGI request");
            responseMsg = cgiHandler_.handleRequest(clientSession);
        } else {
            // 정적 파일 요청
			DEBUG_LOG("[EventHandler]Static file request");
            responseMsg = staticHandler_.handleRequest(requestMsg, reqConfig);
        }
        // 생성된 응답 메시지를 클라이언트 세션의 쓰기 버퍼에 저장
        clientSession.setWriteBuffer(responseMsg);

    } else if (status == REQUEST_ERROR) {
        // 요청 처리 도중 에러가 발생한 경우
        // Http 상태 코드(에러)를 가져와서 에러 응답 세팅
        int statusCode = clientSession.getErrorStatusCode();
        handleError(statusCode, clientSession);
        // 추후 응답 전송 후 연결 종료 처리를 위해 flag 설정
        clientSession.setConnectionClosed();
    } 

    return status;
}

// ─────────────────────────────────────────────────────────────────────────────────────
// 리다이렉션 처리
std::string EventHandler::handleRedirection(const RequestConfig& conf) {
	// 리다이렉션 설정이 올바른 경우
	if (!conf.returnUrl_.empty() && \
	(conf.returnStatus_ == FOUND || conf.returnStatus_ == MOVED_PERMANENTLY)) {
		// 리다이렉트 응답
		DEBUG_LOG("[EventHandler]Redirect to " + conf.returnUrl_)
		std::map<std::string, std::string> headers;
		headers["Location"] = conf.returnUrl_;
		return responseBuilder_.build(conf.returnStatus_, headers, "");
	} else {
		// 그 외에는 에러 응답
		DEBUG_LOG("[EventHandler]Invalid redirection setting")
		return responseBuilder_.buildError(INTERNAL_SERVER_ERROR, conf);
	}
}

//---------------------------------------------------------------------
// 클라이언트 소켓(fd)에서 발생한 쓰기 이벤트 처리
// - 클라이언트에게 응답 데이터를 전송합니다.
//---------------------------------------------------------------------
EnumSesStatus EventHandler::handleClientWriteEvent(ClientSession& clientSession) {
    // 클라이언트에게 응답 전송을 시도하고, 전송 결과 상태를 반환
    EnumSesStatus status = sendResponse(clientSession);
    if (clientSession.isConnectionClosed()) {
        status = CONNECTION_CLOSED;
    }
    return status;
}

//---------------------------------------------------------------------
// 에러 응답 처리 함수
// - 에러 상태 코드에 따라 적절한 에러 응답 메시지를 생성 및 전송합니다.
//---------------------------------------------------------------------
void EventHandler::handleError(int statusCode, ClientSession& clientSession) {
    // ResponseBuilder를 사용하여 상태 코드에 맞는 에러 응답 메시지 생성
    std::string errorMsg = responseBuilder_.buildError(statusCode, *clientSession.getConfig());

    // 생성된 에러 메시지를 클라이언트 세션의 쓰기 버퍼에 저장
    clientSession.setWriteBuffer(errorMsg);
}