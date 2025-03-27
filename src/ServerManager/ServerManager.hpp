#ifndef SERVER_MANAGER_HPP
#define SERVER_MANAGER_HPP

#include "../GlobalConfig/GlobalConfig.hpp"
#include "../Demultiplexer/KqueueDemultiplexer.hpp"
#include "../TimeoutHandler/TimeoutHandler.hpp"
#include "../EventHandler/EventHandler.hpp"
#include "../ClientManager/ClientManager.hpp"
#include "../include/commonEnums.hpp"

#include <set>
#include <vector>

class ServerManager {
    public:
        ~ServerManager();

        // 수신 소켓 설정 함수: 클라이언트 연결 수신을 위한 소켓들을 초기화하고 바인딩하는 역할
        void setupListeningSockets(); 
        // 서버 실행 함수: 메인 이벤트 루프를 돌며 클라이언트 연결 및 데이터 송수신, 타임아웃 관리 등을 수행
        void run();
		// 디버깅을 위한 현재 수신 소켓 정보 출력 함수
		void print() const;

    private:
        // 수신 소켓 파일 디스크립터 집합: 서버가 수신 대기 중인 소켓들을 저장
        std::set<int> listenFds_; 

		// setupListeningSockets() 함수에서 사용되는 함수들
		// 새 TCP 소켓을 생성합니다.
        int createListeningSocket(const ServerConfig &server) const;
        // 주소 정보를 초기화합니다.
        void initAddrInfo(struct addrinfo &hints) const;
        // 소켓을 구성합니다.
        int configureSocket(int sockFd) const;
        // 소켓을 논블로킹 모드로 설정합니다.
        int setNonBlocking(int sockFd) const;
        // 소켓 옵션을 설정합니다.
        int setSocketOptions(int sockFd) const;

		// run() 함수에서 사용되는 함수들
        // 서버 실행 여부 확인 함수: 서버가 정상적으로 실행 중인지 판단
        bool isServerRunning() const;

        // 주어진 파일 디스크립터(fd)가 수신 소켓인지 판단하는 함수
        bool isListeningSocket(int fd);

        // 새로운 클라이언트 연결 정보를 추가하는 함수
        void addClientInfo(int clientFd, Demultiplexer& reactor, TimeoutHandler& timeoutHandler);
        // 기존 클라이언트 연결 정보를 제거하는 함수
        void removeClientInfo(int clientFd, ClientManager& clientManager, TimeoutHandler& timeoutHandler);

        //리스닝 소켓에서 읽기 이벤트가 발생하면 새로운 클라이언트의 연결 요청을 처리
		void processServerReadEvent(int fd, ClientManager& clientManager, \
		EventHandler& eventHandler, TimeoutHandler& timeoutHandler, Demultiplexer& reactor);
        // 클라이언트 소켓에서 읽기 이벤트가 발생한 경우, 클라이언트 데이터를 처리
		void processClientReadEvent(int clientFd, ClientManager& clientManager, \
		EventHandler& eventHandler, TimeoutHandler& timeoutHandler, Demultiplexer& reactor);
        // cgi pipe에서 read 이벤트가 발생한 경우 처리
        void processCgiReadEvent(int pipeFd, ClientManager& clientManager,\
	    EventHandler& eventHandler, TimeoutHandler& timeoutHandler, Demultiplexer& reactor);
        // 클라이언트 소켓에 쓰기 이벤트가 발생한 경우 데이터를 전송
		void processClientWriteEvent(int fd, ClientManager& clientManager, \
		EventHandler& eventHandler, TimeoutHandler& timeoutHandler, Demultiplexer& reactor);
};

#endif
