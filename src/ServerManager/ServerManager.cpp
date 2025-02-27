#include "ServerManager.hpp"

#include <iostream>
#include <string>
#include <map>
#include <utility>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <sstream>
#include "../include/errorUtils.hpp"

// 프로그램 종료 시, 열려있는 소켓들을 닫기 위한 소멸자입니다.
ServerManager::~ServerManager() {
    // listenFds_에 저장된 모든 소켓 파일 디스크립터에 대해 close()를 호출합니다.
    for (std::set<int>::iterator it = listenFds_.begin(); it != listenFds_.end(); ++it) {
        close(*it);
    }
    // 소켓 파일 디스크립터 집합을 비웁니다.
    listenFds_.clear();
}

// 서버 매니저를 초기화하고 수신 소켓을 설정하는 함수입니다.
void ServerManager::setupListeningSockets() {
    // 전역 설정(GlobalConfig) 인스턴스를 가져와 수정할 수 있도록 const_cast를 사용합니다.
    GlobalConfig &globalConfig = const_cast<GlobalConfig&>(GlobalConfig::getInstance());
    // (호스트, 포트) 쌍을 키로, 소켓 파일 디스크립터를 값으로 하는 맵을 생성합니다.
    std::map<std::pair<std::string, int>, int> addressToSocket;

    // 전역 설정에 등록된 각 가상 서버 구성에 대해 반복합니다.
    for (size_t i = 0; i < globalConfig.servers_.size(); ++i) {
        ServerConfig &server = globalConfig.servers_[i];
        // 현재 서버의 호스트와 포트 정보를 키로 생성합니다.
        std::pair<std::string, int> key(server.host_, server.port_);

        int sockFd;
        // 해당 호스트/포트 조합에 대해 이미 소켓이 생성되었는지 확인합니다.
        if (addressToSocket.find(key) != addressToSocket.end()) {
            sockFd = addressToSocket[key];
        } else {
            // 새로운 TCP 소켓을 생성합니다.
            sockFd = createListeningSocket(server);
            // 중복 소켓 생성을 방지하기 위해 맵에 저장합니다.
            addressToSocket[key] = sockFd;
            // 생성된 소켓을 수신 대기 소켓 집합에 추가합니다.
            listenFds_.insert(sockFd);
        }
        // 현재 서버 구성을 해당 수신 소켓에 매핑합니다.
        // 이를 통해 여러 서버 구성이 동일한 소켓을 공유할 수 있습니다.
        std::clog << "Connectected LISTENING SOCKET" << i+1 << " : " << sockFd << std::endl;
        globalConfig.listenFdToServers_[sockFd].push_back(&server);
    }
}

// 서버 구성에 따라 소켓을 생성하고 바인딩하고 수신 대기 상태로 전환하는 함수입니다.
int ServerManager::createListeningSocket(const ServerConfig &server) const {
    int sockFd = -1;
    struct addrinfo hints, *result, *currAddr;
    
    // 주소 정보를 위한 힌트 구조체를 초기화합니다.
    initAddrInfo(hints);
    
    // 포트 번호를 문자열로 변환합니다.
	std::stringstream ss;
	ss << server.port_;
	std::string portStr = ss.str();
	const char* portCStr = portStr.c_str();
    
    // 서버의 호스트명과 포트 번호를 기반으로 주소 정보를 가져옵니다.
	int s = getaddrinfo(server.host_.c_str(), portCStr, &hints, &result);
    if (s != 0) {
        webserv::throwError("getaddrinfo failed", 
                        server.host_ + ":" + portCStr, 
                        std::string("gai_error: ") + gai_strerror(s));
    }

    // 얻어온 주소 정보 리스트를 순회하며 소켓 생성 및 바인딩을 시도합니다.
    for (currAddr = result; currAddr != NULL; currAddr = currAddr->ai_next) {
        // 소켓을 생성합니다.
        sockFd = socket(currAddr->ai_family, currAddr->ai_socktype, currAddr->ai_protocol);
        if (sockFd < 0)
            continue;
        // 소켓 옵션 설정 (주소 재사용 및 논블로킹 모드 설정)
        if (configureSocket(sockFd)) {
            close(sockFd);
            continue;
        }
        // 소켓을 특정 주소와 포트에 바인딩합니다.
        if (bind(sockFd, currAddr->ai_addr, currAddr->ai_addrlen) < 0) {
            close(sockFd);
            continue;
        }
        // 바인딩에 성공하면 반복문을 종료합니다.
        break;
    }
    // 사용한 주소 정보 메모리를 해제합니다.
    freeaddrinfo(result);
    // 모든 주소에 바인딩하지 못한 경우 예외를 발생시킵니다.
    if (currAddr == NULL) {
        throw std::runtime_error("Failed to bind socket to " + server.host_ +
                                 ":" + std::string(portStr));
    }
    // 소켓을 수신 대기 상태로 전환합니다.
    if (listen(sockFd, SOMAXCONN) < 0) {
        close(sockFd);
        throw std::runtime_error("Failed to listen on socket");
    }
    // 생성된 소켓 파일 디스크립터를 반환합니다.
    return sockFd;
}

// addrinfo 구조체의 힌트를 초기화하는 함수입니다.
void ServerManager::initAddrInfo(struct addrinfo &hints) const {
    // hints 구조체의 모든 필드를 0으로 초기화합니다.
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;			// IPv4 허용
    hints.ai_socktype = SOCK_STREAM;	// TCP 스트림 소켓 사용
    hints.ai_flags = AI_PASSIVE;		// 로컬 주소에 바인딩하기 위해 설정
}

// 소켓 옵션과 논블로킹 모드를 설정하는 함수입니다.
int ServerManager::configureSocket(int sockFd) const {
    if (setSocketOptions(sockFd) < 0) {
        return -1;
    }
    if (setNonBlocking(sockFd) < 0) {
        return -1;
    }
    return 0;
}

// 소켓을 논블로킹 모드로 설정하는 함수입니다.
int ServerManager::setNonBlocking(int sockFd) const {
    // 현재 파일 디스크립터의 플래그를 가져옵니다.
    int flags = fcntl(sockFd, F_GETFL, 0);
    if (flags == -1) {
        return -1;
    }
    // 논블로킹 플래그를 추가하여 파일 디스크립터의 모드를 변경합니다.
    if (fcntl(sockFd, F_SETFL, flags | O_NONBLOCK) == -1) {
        return -1;
    }
    return 0;
}

// 로컬 주소 재사용을 허용하는 소켓 옵션을 설정하는 함수입니다.
int ServerManager::setSocketOptions(int sockFd) const {
    int opt = 1;
    // SO_REUSEADDR 옵션을 활성화하여, 소켓을 빠르게 재사용할 수 있도록 합니다.
    if (setsockopt(sockFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        return -1;
    }
    return 0;
}

extern volatile bool globalServerRunning;

// 서버를 종료해야 하는지 확인하는 함수입니다.
bool ServerManager::isServerRunning() const {
	return globalServerRunning;
}

// 디버깅을 위해 현재 수신 소켓 정보를 출력합니다.
void ServerManager::print() const {
	std::cout << "listenFds: ";
	for (std::set<int>::iterator it = listenFds_.begin(); it != listenFds_.end(); ++it) {
		std::cout << *it << ", ";
	}
	std::cout << std::endl;

	std::cout << "listenFdToServers_: " << std::endl;
	for (std::map<int, std::vector<ServerConfig*> >::const_iterator it = GlobalConfig::getInstance().listenFdToServers_.begin();
		 it != GlobalConfig::getInstance().listenFdToServers_.end(); ++it) {
		std::cout << "listenFd: " << it->first << std::endl;
		for (size_t i = 0; i < it->second.size(); ++i) {
			std::cout << "server: " << it->second[i]->host_ << ":" << it->second[i]->port_ << std::endl;
		}
	}
}

// 주어진 파일 디스크립터(fd)가 수신 소켓인지 판단하는 함수
bool ServerManager::isListeningSocket(int fd) {
	return listenFds_.find(fd) != listenFds_.end();
}