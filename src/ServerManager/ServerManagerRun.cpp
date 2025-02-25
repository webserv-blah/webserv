#include "ServerManager.hpp"

// EventLoop 실행 함수
void ServerManager::run() {
	EventHandler 	eventHandler; // 이벤트 처리 담당 객체
	ClientManager	clientManager; // 클라이언트 세션 관리 객체

	try {
		Demultiplexer	reactor(listenFds_); // I/O 멀티플렉싱을 위한 리액터 객체
		TimeoutHandler	timeoutHandler; // 클라이언트 타임아웃 관리 객체

		while (isServerRunning()) {
			int	numEvents = reactor.waitForEvent(); // 발생한 이벤트 개수 확인

			for (int i = 0; i < numEvents; ++i) {
				EnumEvent	type = reactor.getEventType(i);
				int 		fd = reactor.getSocketFd(i);

				if (type == EXCEPTION_EVENT) {
					// 예외 이벤트 발생 시 클라이언트 제거
					removeClientInfo(fd, clientManager, reactor, timeoutHandler);
				} else if (type == READ_EVENT) {
					if (isListeningSocket(fd)) {
						// 리스닝 소켓에서 읽기 이벤트 발생
						processServerReadEvent(
						fd, clientManager, eventHandler, timeoutHandler, reactor);
					} else {
						// 클라이언트 소켓에서 읽기 이벤트 발생
						processClientReadEvent(
						fd, clientManager, eventHandler, timeoutHandler, reactor);
					}
				} else if (type == WRITE_EVENT) {
					// 쓰기(송신) 이벤트 발생
					processClientWriteEvent(
					fd, clientManager, eventHandler, timeoutHandler, reactor);
				}
			}
			// 타임아웃된 클라이언트 확인 및 처리
			timeoutHandler.checkTimeouts(eventHandler, reactor, clientManager);
		}
	} catch (std::exception& e) {
		// 서버 비정상 종료 시, 연결된 클라이언트에 서버 종료 안내 및 에러 throw
		notifyClientsShutdown(clientManager, eventHandler);
		throw ;
	}

	// 서버 종료 시, 연결된 클라이언트에 서버 종료 안내
	notifyClientsShutdown(clientManager, eventHandler);
}

// 서버 종료 전, 모든 클라이언트에 종료 응답을 전송
void ServerManager::notifyClientsShutdown(ClientManager& clientManager, EventHandler& eventHandler) {
	ClientManager::TypeClientMap&			clientList = clientManager.accessClientSessionMap();
	ClientManager::TypeClientMap::iterator	it;

	for (it = clientList.begin(); it != clientList.end(); ++it) {
		eventHandler.handleError(SERVICE_UNAVAILABLE, *it->second); // 클라이언트에게 서버 종료 알림
	}
}

// 새로운 클라이언트 정보 추가 함수
void ServerManager::addClientInfo(int clientFd, Demultiplexer& reactor, TimeoutHandler& timeoutHandler) {
	timeoutHandler.addConnection(clientFd); // 타임아웃 관리 추가
	reactor.addSocket(clientFd);			// 리액터(이벤트 루프)에 소켓 등록
}

// 기존 클라이언트 정보 삭제 함수
void ServerManager::removeClientInfo(int clientFd, ClientManager& clientManager, Demultiplexer& reactor, TimeoutHandler& timeoutHandler) {
	clientManager.removeClient(clientFd);		// 클라이언트 제거
	timeoutHandler.removeConnection(clientFd);	// 타임아웃 관리 제거
	reactor.removeSocket(clientFd);				// 리액터에서 소켓 제거
}

void ServerManager::processServerReadEvent(int fd, ClientManager& clientManager, \
EventHandler& eventHandler, TimeoutHandler& timeoutHandler, Demultiplexer& reactor) {
	int clientFd = eventHandler.handleServerReadEvent(fd, clientManager);

	if (clientFd > 0) { // 새로운 클라이언트가 정상적으로 연결됨
		addClientInfo(clientFd, reactor, timeoutHandler);
	}
}

void ServerManager::processClientReadEvent(int fd, ClientManager& clientManager, \
EventHandler& eventHandler, TimeoutHandler& timeoutHandler, Demultiplexer& reactor) {
	ClientSession*	client = clientManager.accessClientSession(fd);
	if (!client) {
		std::cerr 
		<< "[WARNING] Invalid Value - No clientSession corresponding to fd(source: ServerManager::processClientReadEvent())"
		<< std::endl;
		return ;
	}

	EnumSesStatus	status = eventHandler.handleClientReadEvent(*client);
	if (status == CONNECTION_CLOSED) { // 클라이언트가 연결 종료
		removeClientInfo(fd, clientManager, reactor, timeoutHandler);
	} else if (status == WRITE_CONTINUE) { // 추가적인 쓰기 작업 필요
		timeoutHandler.updateActivity(fd); // 타임아웃 갱신
		reactor.addWriteEvent(fd); // 쓰기 이벤트 추가
	} else { // 데이터 수신 후 타임아웃 갱신
		timeoutHandler.updateActivity(fd);
	}
}

void ServerManager::processClientWriteEvent(int fd, ClientManager& clientManager, \
EventHandler& eventHandler, TimeoutHandler& timeoutHandler, Demultiplexer& reactor) {
	ClientSession*	client = clientManager.accessClientSession(fd);
	if (!client) {
		std::cerr 
		<< "[WARNING] Invalid Value - No clientSession corresponding to fd(source: ServerManager::processClientWriteEvent())"
		<< std::endl;
		return ;
	}

	EnumSesStatus	status = eventHandler.handleClientWriteEvent(*client);
	if (status == CONNECTION_CLOSED) { // 클라이언트가 연결 종료
		removeClientInfo(fd, clientManager, reactor, timeoutHandler);
	} else if (status == WRITE_COMPLETE) { // 데이터 전송 완료
		reactor.removeWriteEvent(fd); // 쓰기 이벤트 제거
	}
}


