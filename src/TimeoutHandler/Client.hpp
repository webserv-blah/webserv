#ifndef CLIENT_HPP
#define CLIENT_HPP
#include <map>

// ClientSession 인터페이스의 최소한의 구현체
class ClientSession {
public:
    int fd;
    ClientSession(int fd_) : fd(fd_) {}
};

// ClientManager 인터페이스의 최소한의 구현체
class ClientManager {
public:
    std::map<int, ClientSession*> clients;

    ClientSession* accessClientSession(int fd) {
        if (clients.find(fd) != clients.end())
            return clients[fd];
        return nullptr;
    }

    void removeClient(int fd) {
        auto it = clients.find(fd);
        if (it != clients.end()) {
            delete it->second;
            clients.erase(it);
        }
    }

    // 테스트를 위해 수동으로 ClientSession 추가
    void addClient(int fd) {
        clients[fd] = new ClientSession(fd);
    }

    ~ClientManager() {
        // 메모리 해제
        for (auto& p : clients) {
            delete p.second;
        }
    }
};


#endif