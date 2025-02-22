#ifndef SERVER_MANAGER_HPP
# define SERVER_MANAGER_HPP

# include <set>
# include "../GlobalConfig/GlobalConfig.hpp"
# include "../Demultiplexer/KqueueDemultiplexer.hpp"
# include "../TimeoutHandler/TimeoutHandler.hpp"
# include "../EventHandler/EventHandler.hpp"
# include "../ClientManager/ClientManager.hpp"
# include "../include/commonEnums.hpp"

class ServerManager {
	public:
		~ServerManager(); // 소멸자: 객체 소멸 시 리소스 정리 등 필요한 작업 수행

		void setupListeningSockets(); 
        // 수신 소켓 설정 함수: 클라이언트 연결 수신을 위한 소켓들을 초기화하고 바인딩하는 역할

		void run();
        // 서버 실행 함수: 메인 이벤트 루프를 돌며 클라이언트 연결 및 데이터 송수신, 타임아웃 관리 등을 수행

		bool isServerRunning();
        // 서버 실행 여부 확인 함수: 서버가 정상적으로 실행 중인지 판단

	private:
		std::set<int> listenFds_; 
        // 수신 소켓 파일 디스크립터 집합: 서버가 수신 대기 중인 소켓들을 저장

		// bool listenFdStatus_; // signal 핸들링과 관련 있으므로 논의 필요

		bool isListeningSocket(int fd);
        // 주어진 파일 디스크립터(fd)가 수신 소켓인지 판단하는 함수

		void addClientInfo(int listenFd, int clientFd, ClientManager& clientManager, Demultiplexer& reactor, TimeoutHandler& timeoutHandler);
        // 새로운 클라이언트 연결 정보를 추가하는 함수
        // - ClientManager: 클라이언트 세션 관리
        // - Demultiplexer: 이벤트 루프에 클라이언트 소켓 등록
        // - TimeoutHandler: 클라이언트 타임아웃 관리 추가

		void removeClientInfo(int clientFd, ClientManager& clientManager, Demultiplexer& reactor, TimeoutHandler& timeoutHandler);
        // 기존 클라이언트 연결 정보를 제거하는 함수
        // - ClientManager: 클라이언트 세션 제거
        // - Demultiplexer: 이벤트 루프에서 클라이언트 소켓 제거
        // - TimeoutHandler: 타임아웃 관리 제거

		void cleanUpConnections(ClientManager& clientManager, EventHandler& eventHandler);
        // 서버 종료 시, 모든 클라이언트에 종료 응답 전송 및 연결 정리 함수
        // - ClientManager: 모든 클라이언트 세션 접근
        // - EventHandler: 클라이언트에게 서버 종료 알림 전송
};

#endif
