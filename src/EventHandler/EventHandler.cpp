#include "EventHandler.hpp"
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "../include/errorUtils.hpp"

// ResponseBuilder를 인자로 하여 정적 핸들러와 CGI 핸들러를 초기화합니다.
EventHandler::EventHandler() : staticHandler_(rspBuilder_), cgiHandler_(rspBuilder_) {

}

EventHandler::~EventHandler() {

}

//---------------------------------------------------------------------
// 서버 소켓(fd)에서 발생한 읽기 이벤트 처리
// - 클라이언트 연결 요청을 수락하고, 클라이언트의 파일 디스크립터를 반환합니다.
// - 연결 수락에 실패하면 -1을 반환합니다.
//---------------------------------------------------------------------
int EventHandler::handleServerReadEvent(int fd, ClientManager& clientManager) {
    DEBUG_LOG("[EventHandler] Handling server read event on listen fd: " << fd);
    
    struct sockaddr_in clientAddr;              // 클라이언트 주소 정보(IPv4)
    socklen_t addrLen = sizeof(clientAddr);     // 클라이언트 주소 구조체의 크기

    // accept() 호출을 통해 클라이언트 연결 수락
    int clientFd = accept(fd, reinterpret_cast<struct sockaddr*>(&clientAddr), &addrLen);
    if (clientFd < 0) {
        // 연결 수락 실패 - 시스템 호출 에러를 로깅
        webserv::logSystemError(ERROR, "accept", 
                              "Server fd: " + std::to_string(fd), 
                              "EventHandler::handleServerReadEvent");
        return -1;
    }

    // 클라이언트의 IP 주소 문자열로 변환
    std::string clientIP(inet_ntoa(clientAddr.sin_addr));
    
    DEBUG_LOG("[EventHandler] New connection accepted from " << clientIP << " assigned to fd: " << clientFd);

    // ClientManager에 새 클라이언트 정보를 추가
    clientManager.addClient(fd, clientFd, clientIP);
    
    // 클라이언트 세션에 설정 적용
    ClientSession* session = clientManager.accessClientSession(clientFd);
    if (session != NULL) {
        // fd에 해당하는 서버 설정 찾기
        const GlobalConfig &globalConfig = GlobalConfig::getInstance();
        const RequestConfig* defaultConfig = globalConfig.findRequestConfig(fd, "", "");
        if (defaultConfig != NULL) {
            session->setConfig(defaultConfig);
            DEBUG_LOG("[EventHandler] Set initial server config for new client fd:" << clientFd);
        }
    }

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
    DEBUG_LOG("[EventHandler] Handling client read event on fd: " << clientSession.getClientFd());
    
    // 클라이언트의 요청 데이터를 수신
    EnumSesStatus status = recvRequest(clientSession);
    DEBUG_LOG("[EventHandler] recvRequest result status: " << status);

    if (status == READ_COMPLETE) {
        // 요청 데이터 수신이 완료된 경우
        if (clientSession.getReqMsgPtr() == NULL) {
            DEBUG_LOG("[EventHandler] Request message is NULL after READ_COMPLETE, possible error");
            handleError(INTERNAL_SERVER_ERROR, clientSession);
            return CONNECTION_CLOSED;
        }
        
        RequestMessage  requestMsg = clientSession.getReqMsg();
        std::string     responseMsg;
        
        // 설정이 NULL인지 확인
        if (clientSession.getConfigPtr() == NULL) {
            DEBUG_LOG("[EventHandler] Config is NULL after READ_COMPLETE, using default config");
            const GlobalConfig &globalConfig = GlobalConfig::getInstance();
            const RequestConfig* defaultConfig = globalConfig.findRequestConfig(clientSession.getListenFd(), "", "");
            if (defaultConfig != NULL) {
                clientSession.setConfig(defaultConfig);
            } else {
                DEBUG_LOG("[EventHandler] Could not find default config, sending 500 error");
                handleError(INTERNAL_SERVER_ERROR, clientSession);
                return CONNECTION_CLOSED;
            }
        }
        
        const RequestConfig& reqConfig = clientSession.getConfig();
        DEBUG_LOG("[EventHandler] Processing request with URI: " << requestMsg.getTargetURI());

        // 요청 URI에 CGI 실행 대상이 포함되어 있는지 확인
        if (cgiHandler_.isCGI(requestMsg.getTargetURI(), reqConfig.cgiExtension_)) {
            // CGI 요청
            DEBUG_LOG("[EventHandler] Processing CGI request for URI: " << requestMsg.getTargetURI());
            responseMsg = cgiHandler_.handleRequest(clientSession);
        } else {
            // 정적 파일 요청
            DEBUG_LOG("[EventHandler] Processing static file request for URI: " << requestMsg.getTargetURI());
            responseMsg = staticHandler_.handleRequest(requestMsg, reqConfig);
        }
        
        // 생성된 응답 메시지를 클라이언트 세션의 쓰기 버퍼에 저장
        clientSession.setWriteBuffer(responseMsg);
        DEBUG_LOG("[EventHandler] Response created, size: " << responseMsg.size() << " bytes");
        
        // 요청 처리가 완료되었으므로 요청 객체 초기화
        clientSession.resetRequest();
        
        // 클라이언트에게 응답 전송을 시도하고, 전송 결과에 따라 상태 갱신
        status = sendResponse(clientSession);
        DEBUG_LOG("[EventHandler] sendResponse result status: " << status);
    } else if (status == REQUEST_ERROR) {
        // 요청 처리 도중 에러가 발생한 경우
        // Http 상태 코드(에러)를 가져와서 에러 응답 전송
        int statusCode = clientSession.getErrorStatusCode();
        DEBUG_LOG("[EventHandler] Request error detected, status code: " << statusCode << " for client fd: " << clientSession.getClientFd());
        handleError(statusCode, clientSession);
        status = CONNECTION_CLOSED;
    } 

    return status;
}

//---------------------------------------------------------------------
// 클라이언트 소켓(fd)에서 발생한 쓰기 이벤트 처리
// - 클라이언트에게 응답 데이터를 전송합니다.
//---------------------------------------------------------------------
EnumSesStatus EventHandler::handleClientWriteEvent(ClientSession& clientSession) {
    DEBUG_LOG("[EventHandler] Handling client write event on fd: " << clientSession.getClientFd());
    // 클라이언트에게 응답 전송을 시도하고, 전송 결과 상태를 반환
    EnumSesStatus status = sendResponse(clientSession);
    
    return status;
}

//---------------------------------------------------------------------
// 에러 응답 처리 함수
// - 에러 상태 코드에 따라 적절한 에러 응답 메시지를 생성 및 전송합니다.
//---------------------------------------------------------------------
void EventHandler::handleError(int statusCode, ClientSession& clientSession) {
    DEBUG_LOG("[EventHandler] Building error response for status code: " << statusCode << " for client fd: " << clientSession.getClientFd());
    
    // 클라이언트 세션에 설정이 있는지 확인
    const RequestConfig* configPtr = clientSession.getConfigPtr();
    std::string errorMsg;
    
    // 설정이 없으면 기본 서버 설정 사용
    if (configPtr == NULL) {
        DEBUG_LOG("[EventHandler] No client config found, using default server config for error response");
        
        // 기본 서버 설정 가져오기 시도
        const GlobalConfig &globalConfig = GlobalConfig::getInstance();
        const RequestConfig* defaultConfig = globalConfig.findRequestConfig(clientSession.getListenFd(), "", "");
        if (defaultConfig != NULL) {
            clientSession.setConfig(defaultConfig);
            configPtr = clientSession.getConfigPtr();
        }
        
        // 여전히 설정이 없으면 임시 설정 사용
        if (configPtr == NULL) {
            RequestConfig defaultConfig;
            errorMsg = rspBuilder_.buildError(statusCode, defaultConfig);
        } else {
            errorMsg = rspBuilder_.buildError(statusCode, *configPtr);
        }
    } else {
        // 클라이언트에 설정된 정보로 에러 응답 생성
        errorMsg = rspBuilder_.buildError(statusCode, *configPtr);
    }

    // 생성된 에러 메시지를 클라이언트 세션의 쓰기 버퍼에 저장
    clientSession.setWriteBuffer(errorMsg);

    // 클라이언트에게 에러 응답 전송
    sendResponse(clientSession);
}