#include "TimeoutHandler.hpp"

TimeoutHandler::TimeoutHandler() {

}

TimeoutHandler::~TimeoutHandler() {

}

//TimeoutHandler 객체에서 관리할 연결시간 정보 추가
void    TimeoutHandler::addConnection(int fd) {
    time_t lastActivity = getTime();
    connections_[fd] = lastActivity;

    time_t  expire = lastActivity + LIMIT;
    ExpireQueueIter it = expireQueue_.insert(std::make_pair(expire, fd));
    expireMap_[fd] = it;
}

//최근 활동 시각 및 만료 시각 업데이트
void    TimeoutHandler::updateActivity(int fd) {
    ConnectionIter cit = connections_.find(fd);
    if (cit == connections_.end()) {
        return ;
    }
    
    cit->second = getTime();

    ExpireIterMapIter eit = expireMap_.find(fd);
    if (eit != expireMap_.end()) { //존재할 경우
        expireQueue_.erase(eit->second);
        expireMap_.erase(eit);
    }

    time_t expire = cit->second + LIMIT;
    ExpireQueueIter qit = expireQueue_.insert(std::make_pair(expire, fd));
    expireMap_[fd] = qit;
}

//만료된 연결 처리
void    TimeoutHandler::checkTimeouts(EventHandler& eventHandler, Demultiplexer& reactor, ClientManager& clientManager) {
    time_t currentTime = getTime();

    while (!expireQueue_.empty()) {
        ExpireQueueIter it = expireQueue_.begin();

        if (it->first > currentTime) { //만료 X
            return ;
        }

        int fd = it->second;
        
		//send 408 error
		//실제 handleError 구현 후 인자 수정 예정
		eventHandler.handleError(408); 

		//client 정보 삭제
        removeConnection(fd, it);
		reactor.removeSocket(fd);
		clientManager.removeClient(fd); // Q. 내부에서 close(fd)할건지
    }
}

//연결 정보 제거(by fd) - 주로 객체 외부에서 사용
void TimeoutHandler::removeConnection(int fd) {
    ExpireQueueIter it = expireMap_[fd];

    connections_.erase(fd);
    expireMap_.erase(fd);
    expireQueue_.erase(it);
}

//연결 정보 제거(by fd & iterator) - 객체 내부(checkTimeouts)에서 사용
void TimeoutHandler::removeConnection(int fd, ExpireQueueIter it) {
    connections_.erase(fd);
    expireMap_.erase(fd);
    expireQueue_.erase(it);
}

//현재 시각(초) 반환
time_t  TimeoutHandler::getTime() const {
    return time(NULL);
}
