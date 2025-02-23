#ifndef TIMEOUT_HANDLER_HPP
#define TIMEOUT_HANDLER_HPP

//테스트용 dummy includes
#include "EventHandler.hpp"
// #include "KqueueDemultiplexer.hpp"
#include "Demultiplexer.hpp"
// #include "ClientManager.hpp"
#include "Client.hpp"
#include <map>

static const int LIMIT = 2;//테스트용 만료시간

class TimeoutHandler {
    public:
        typedef std::map<int, time_t>::iterator         TypeConnectionIter;
        typedef std::multimap<time_t, int>::iterator    TypeExpireQueueIter;
        typedef std::map<int, TypeExpireQueueIter>      TypeExpireIterMap;
        typedef TypeExpireIterMap::iterator             TypeExpireIterMapIter;
        
        TimeoutHandler();
        ~TimeoutHandler();

        void addConnection(int fd);
        void updateActivity(int fd);
        void checkTimeouts(EventHandler& eventHandler, Demultiplexer& reactor, ClientManager& clientManager);
        void removeConnection(int fd);

    private:
        std::map<int, time_t>       connections_; // 연결 정보 (fd -> 마지막 활동 시간)
        std::multimap<time_t, int>  expireQueue_; // 만료 시간 기준으로 오름차순 정렬된 연결 목록
        TypeExpireIterMap           expireMap_;   // fd -> expireQueue_의 iterator
												  // expireQueue_에 fd값으로 빠르게 접근하기 위한 자료구조

        void removeConnection(int fd, TypeExpireQueueIter it);
        time_t getTime() const;

};

#endif
