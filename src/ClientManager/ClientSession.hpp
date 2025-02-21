// ClientSession.hpp (dummy 구현)
#ifndef CLIENT_SESSION_HPP
#define CLIENT_SESSION_HPP

#include <iostream>

class ClientSession {
public:
    // 동적 할당/해제 테스트를 위한 카운터
    static int counter;

    ClientSession(int listenFd, int clientFd)
        : listenFd_(listenFd), clientFd_(clientFd) {
        ++counter;
    }

    ~ClientSession() {
        --counter;
    }

    int getListenFd() const { return listenFd_; }
    int getClientFd() const { return clientFd_; }

private:
    int listenFd_;
    int clientFd_;
};


#endif // CLIENT_SESSION_HPP
