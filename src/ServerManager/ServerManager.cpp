#include "ServerManager.hpp"

#include <iostream>
#include <string>
#include <map>
#include <utility>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

void ServerManager::~ServerManager() {
    for (std::set<int>::iterator it = listenFds_.begin(); it != listenFds_.end(); ++it) {
        close(*it);
    }
    listenFds_.clear();
}

// 서버 매니저를 초기화하고 수신 소켓을 설정합니다.
void ServerManager::setupListeningSockets() {
	// 전역 설정 인스턴스를 가져옵니다.
	const GlobalConfig& globalConfig = GlobalConfig::getInstance();
    // 호스트/포트 쌍과 해당 소켓 파일 디스크립터를 연관짓는 맵입니다.
    std::map<std::pair<std::string, int>, int> addressToSocket;

    // 전역 설정에 정의된 각 가상 서버 구성을 반복합니다.
    for (size_t i = 0; i < globalConfig.servers_.size(); ++i) {
        ServerConfig& server = globalConfig.servers_[i];

        // 이 서버의 호스트와 포트를 가져옵니다.
        std::pair<std::string, int> key = std::make_pair(server.host_, server.port_);

        int sockFd;
        // 이 호스트/포트 조합에 대한 소켓이 이미 생성되었는지 확인합니다.
        if (addressToSocket.find(key) != addressToSocket.end()) {
            sockFd = addressToSocket[key];
        } else {
            // 새 TCP 소켓을 생성합니다.
            sockFd = socket(AF_INET, SOCK_STREAM, 0);
            if (sockFd < 0) {
				throw std::runtime_error("Failed to create socket");
            }

            // 소켓을 논블로킹 모드로 설정합니다.
            // 소켓의 현재 플래그를 가져옵니다.
            int flags = fcntl(sockFd, F_GETFL, 0);
            if (flags == -1) {
                close(sockFd);
                throw std::runtime_error("Failed to get socket flags");
            }
            // 논블로킹 작업을 위해 O_NONBLOCK 플래그를 추가합니다.
            if (fcntl(sockFd, F_SETFL, flags | O_NONBLOCK) == -1) {
                close(sockFd);
                throw std::runtime_error("Failed to set socket flags");
            }

            // 로컬 주소의 재사용을 허용하도록 소켓 옵션을 설정합니다.
            int opt = 1;
            if (setsockopt(sockFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
                close(sockFd);
                throw std::runtime_error("Failed to set socket options");
            }

            // 서버의 주소를 정의하기 위해 sockaddr_in 구조체를 초기화합니다.
            sockaddr_in addr;
            addr.sin_family = AF_INET;                 // IPv4 사용.
            addr.sin_port = htons(server.port_);       // 포트를 설정하며, 네트워크 바이트 순서로 변환.
            if (server.host_ == "0.0.0.0") {
                addr.sin_addr.s_addr = INADDR_ANY;       // 모든 사용 가능한 인터페이스에 바인딩.
            } else {
                // 텍스트 형태의 IP 주소를 바이너리 형태로 변환합니다.
                if (inet_aton(server.host_.c_str(), &addr.sin_addr) == 0) {
                    close(sockFd);
					throw std::runtime_error("Invalid IP address");
                }
            }

            // 지정된 IP 주소와 포트에 소켓을 바인딩합니다.
            if (bind(sockFd, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) < 0) {
                close(sockFd);
				throw std::runtime_error("Failed to bind socket");
            }

            // 소켓에서 들어오는 연결 요청을 수신하기 시작합니다.
            if (listen(sockFd, SOMAXCONN) < 0) {
                close(sockFd);
                throw std::runtime_error("Failed to listen on socket");
            }

            // 중복된 소켓 생성을 방지하기 위해 새로운 소켓을 맵에 저장합니다.
            addressToSocket[key] = sockFd;
            // 소켓을 수신 대기 파일 디스크립터 집합에 추가합니다.
            listenFds_.insert(sockFd);
        }

        // 이 서버 구성을 해당 수신 소켓에 매핑합니다.
        // 이를 통해 여러 서버 구성이 동일한 소켓을 공유할 수 있습니다.
        globalConfig.listenFdToServers_[sockFd].push_back(&server);
    }
}
