#include "TimeoutHandler.hpp"
#include <iostream>
#include "errorUtils.hpp"

TimeoutHandler::TimeoutHandler() {

}

TimeoutHandler::~TimeoutHandler() {

}

// 이벤트루프의 타임아웃 주기 관리를 위해 가장 임박한 만료시간 반환 메소드
timespec* TimeoutHandler::getEarliestTimeout() {
    if (expireQueue_.empty()) {
        return NULL;
    }

    // 만약 관리중인 클라이언트 존재 시, 가장 임박한 만료시간까지 남은 시간 세팅
    time_t  currentTime = getTime();
    time_t  expireTime = expireQueue_.begin()->first;

    timeout_.tv_sec = expireTime - currentTime;  
    timeout_.tv_nsec = 0;
    return &timeout_;
}

// TimeoutHandler 객체에서 관리할 클라이언트(fd) 연결시간 정보 추가
void TimeoutHandler::addConnection(int fd) {
    time_t lastActivity = getTime();
    connections_[fd] = lastActivity;

    time_t expire = lastActivity + IDLE_LIMIT;
    TypeExpireQueueIter it = expireQueue_.insert(std::make_pair(expire, fd));
    expireMap_[fd] = it;
}

// 클라이언트(fd)의 최근 활동 시각 및 만료 시각 업데이트
void TimeoutHandler::updateActivity(int fd, EnumSesStatus status) {
	// fd에 대응하는 연결 정보 찾기
    TypeConnectionIter cit = connections_.find(fd);
    if (cit == connections_.end()) { // 연결 정보가 존재하지 않을 경우
        std::cerr << "[Error][TimeoutHandler][updateActivity] Connection Not Found" << std::endl;
        return;
    }
    
    // fd의 마지막 활동 시각 업데이트
    cit->second = getTime();

    TypeExpireIterMapIter eit = expireMap_.find(fd);
    if (eit != expireMap_.end()) { // 기존 만료 정보 존재할 경우 제거(중복 방지)
        expireQueue_.erase(eit->second);
        expireMap_.erase(eit);
    }

    // 새로운 만료 정보 등록
    time_t limit = (status == READ_CONTINUE) ? REQ_LIMIT : IDLE_LIMIT;
    time_t newExpire;
    newExpire = cit->second + limit;
    TypeExpireQueueIter qit = expireQueue_.insert(std::make_pair(newExpire, fd));
    expireMap_[fd] = qit;
}

// 만료된 클라이언트 정보 정리
// eventHandler, reactor, clientManager : isReciving여부 판별 및 408 Request Timeout 응답 전송을 위한 객체
void TimeoutHandler::checkTimeouts(EventHandler& eventHandler, Demultiplexer& reactor, ClientManager& clientManager) {
    time_t currentTime = getTime();

	DEBUG_LOG("[TimeoutHandler]Checking Timeouts : " << currentTime)

    while (!expireQueue_.empty()) {
        TypeExpireQueueIter it = expireQueue_.begin();

        // 만료된 연결이 없을 경우 종료
        if (it->first > currentTime) {
            return;
        }

        // 만료된 연결 응답처리
        int fd = it->second;
		DEBUG_LOG("[TimeoutHandler]Connection Expired : " << fd)

        ClientSession* client = clientManager.accessClientSession(fd);
        if (!client) { 
            std::cerr << "[Error][TimeoutHandler][checkTimeouts] Invalid Client Fd" << std::endl;
        } else if (client->isReceiving()) {
            // Request Timeout일 경우 HTTP Timeout 처리
            if (client->accessCgiProcessInfo().isProcessing) {
                eventHandler.handleError(504, *client);
                client->accessCgiProcessInfo().isProcessing = 0;
            } else {
                eventHandler.handleError(408, *client);
            }
            reactor.addWriteEvent(fd);
        } else {
            //IDLE Timeout일 경우 나머지 리소스도 정리
            clientManager.removeClient(fd);
            reactor.removeSocket(fd);
        }

        // TimeoutHandler에서 관리하는 client 정보 삭제 및 정리
        removeConnection(fd, it);
    }
}

// 내부에서 사용하는 연결 정보 제거 (by fd & iterator)
void TimeoutHandler::removeConnection(int fd, TypeExpireQueueIter it) {
    connections_.erase(fd);
    expireMap_.erase(fd);
    expireQueue_.erase(it);
}

// 외부에서 호출하는 연결 정보 제거 (by fd)
void TimeoutHandler::removeConnection(int fd) {
	TypeExpireIterMapIter eit = expireMap_.find(fd);
    if (eit == expireMap_.end()) { // 클라이언트(fd)의 만료 정보가 없을 경우
        std::cerr << "[Error][TimeoutHandler][removeConnection] Fd Not Found" << std::endl;
        return ;
    }
    TypeExpireQueueIter it = eit->second; 
    removeConnection(fd, it);
}

// 현재 시각(초 단위) 반환
time_t TimeoutHandler::getTime() const {
    return time(NULL);
}
