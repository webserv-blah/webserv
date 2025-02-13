#pragma once

# include <map>

# define LIMIT 60 //sec

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
        void checkTimeOuts();
        void removeConnection(int fd);

    private:
        std::map<int, time_t>       connections_;
        std::multimap<time_t, int>  expireQueue_;
        ExpireIterMap               expireMap_;
        
        void removeConnection(int fd, ExpireQueueIter it);
        time_t getTime() const;
};
