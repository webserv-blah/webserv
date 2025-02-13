#include "TimeoutHandler.hpp"


TimeoutHandler::TimeoutHandler() {

}

TimeoutHandler::~TimeoutHandler() {

}

void    TimeoutHandler::addConnection(int fd) {
    time_t lastActivity = getTime();
    connections_[fd] = lastActivity;

    time_t  expire = lastActivity + LIMIT;
    ExpireQueueIter it = expireQueue_.insert(std::make_pair(expire, fd));
    expireMap_[fd] = it;
}

void    TimeoutHandler::updateActivity(int fd) {
    ConnectionIter cit = connections_.find(fd);
    if (cit == connections_.end()) {
        // connection not found
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

void    TimeoutHandler::checkTimeOuts() {
    time_t currentTime = getTime();

    while (!expireQueue_.empty()) {
        ExpireQueueIter it = expireQueue_.begin();

        if (it->first > currentTime) { //만료 X
            return ;
        }

        // 만료 처리
        //Q. eventHandler 어떻게 들고 있을 건지. 
        //+ 그 외 clientSocket, demultiplexer에서의 제거 처리
        int fd = it->second;
        {
            //send 408 error
            // eventHandler.handleError(408);
            
            // clean up clientSocket info

        }
        removeConnection(fd, it);
    }
}

void TimeoutHandler::removeConnection(int fd) {
    ExpireQueueIter it = expireMap_[fd];

    connections_.erase(fd);
    expireMap_.erase(fd);
    expireQueue_.erase(it);
}

void TimeoutHandler::removeConnection(int fd, ExpireQueueIter it) {
    connections_.erase(fd);
    expireMap_.erase(fd);
    expireQueue_.erase(it);
}

time_t  TimeoutHandler::getTime() const {
    return time(NULL); //sec.
}

