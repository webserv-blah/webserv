#include "ServerManager.hpp"

// EventLoop
void ServerManager::run() {
	Demultiplexer	reactor(serverFds_);
	EventHandler 	eventHandler();
	TimeoutHandler	timeoutHandler();
	ClientManager	clientManager();

	while (isRunning()) { //g_signal 변수 명을 isRunning으로 둬도 될 듯
		int	numEvents = reactor.waitForEvent();

		for (int i = 0; i < numEvents; ++i) {
			TypeEvent	type = reactor.getEventType(i);
			int 		fd = reactor.getSocketFd(i);

			if (type == EXCEPTION_EVENT) {
				// error response 전송 여부 판단 후 추가
				removeClientInfo(fd, clientManager, reactor, timeoutHandler);
			} else if (type == READ_EVENT) {
				if (isServer(fd)) {
					int clientFd = eventHandler.handleServerReadEvent(fd);

					if (clientFd) {
						addClientInfo(clientFd, clientManager, reactor, timeoutHandler);
					}

				} else {
					TypeSesStatus	status = eventHandler.handleClientReadEvent(clientManager.accessClientSession(fd));

					if (status == CONNECTION_CLOSED) {
						removeClientInfo(fd, clientManager, reactor, timeoutHandler);
					} else if (status == CONNECTION_ERROR) {
						// connection error 처리 로직 추가
					} else if (status == WRITE_CONTINUE) {
						timeoutHandler.updateActivity(fd);
						reactor.addWriteEvent(fd);
					} else {
						timeoutHandler.updateActivity(fd);
					}
					
				}

			} else if (type == WRITE_EVENT) {
				TypeSesStatus	status = eventHandler.handleClientWriteEvent(clientManager.accessClientSession(fd));

				if (status == CONNECTION_CLOSED) {
					removeClientInfo(fd, clientManager, reactor, timeoutHandler);
				} else if (status == WRITE_COMPLETE) {
					timeoutHandler.updateActivity(fd); // 지울지 여부 판단 필요
					reactor.removeWriteEvent(fd);
				} else {
					timeoutHandler.updateActivity(fd);
				}

			} 
		}
		timeoutHandler.checkTimeouts(eventHandler, reactor, clientManager);
	}
	cleanUpConnections(clientManager, eventHandler);
}

// 서버 종료 전, 연결된 클라이언트에 종료 응답 반환 + client fd 및 clientSession 리소스 제거
void ServerManager::cleanUpConnections(ClientManager& clientManager, eventHandler& eventHandler) {
	std::map<int, ClientSession*>			clientList = clientManager.accessClientSessionMap();
	std::map<int, ClientSession*>::iterator	it;

	for (it = clientList.begin(); it != clientList.end(); ) {
		eventHandler.handleServerShutDown(*it->second);
		it = clientManager.removeClient(it->first);
	}
}

// ServerManager의 멤버 함수로 둘지, 별도의 함수로 둘지
// client 정보 추가
void ServerManager::addClientInfo(int clientFd, ClientManager& clientManager, Demultiplexer& reactor, TimeoutHandler& timeoutHandler) {
	clientManager.addClient(clientFd);
	timeoutHandler.addConnection(clientFd);
	reactor.addSocket(clientFd);
}

// client 정보 삭제
void ServerManager::removeClientInfo(int clientFd, ClientManager& clientManager, Demultiplexer& reactor, TimeoutHandler& timeoutHandler) {
	clientManager.removeClient(clientFd);
	timeoutHandler.removeConnection(clientFd);
	reactor.removeSocket(clientFd);
}