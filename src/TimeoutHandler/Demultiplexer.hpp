#ifndef DEMULTIPLEXER_HPP
#define DEMULTIPLEXER_HPP

class Demultiplexer {
    public:
        virtual void removeSocket(int clientFd) = 0;
};

#endif