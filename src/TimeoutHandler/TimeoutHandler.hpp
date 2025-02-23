#ifndef TIMEOUT_HANDLER_HPP
#define TIMEOUT_HANDLER_HPP

#include <map>

#include "../EventHandler/EventHandler.hpp"
#include "../Demultiplexer/KqueueDemultiplexer.hpp"
#include "../ClientManager/ClientManager.hpp"

#define LIMIT 60 //sec => 만료 시간 논의

class TimeoutHandler {
    public:
        typedef std::map<int, time_t>::iterator         ConnectionIter;
        typedef std::multimap<time_t, int>::iterator    ExpireQueueIter;
        typedef std::map<int, ExpireQueueIter>          ExpireIterMap;
        typedef ExpireIterMap::iterator                 ExpireIterMapIter;
        
        TimeoutHandler();
        ~TimeoutHandler();
        
        void addConnection(int fd);
        void updateActivity(int fd);
        void checkTimeouts(EventHandler& eventHandler, Demultiplexer& reactor, ClientManager& clientManager);
        void removeConnection(int fd);

    private:
        std::map<int, time_t>       connections_;
        std::multimap<time_t, int>  expireQueue_;
        ExpireIterMap               expireMap_;
        
        void removeConnection(int fd, ExpireQueueIter it);
        time_t getTime() const;
};

#endif
