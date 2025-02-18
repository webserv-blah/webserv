#include "ServerManager.hpp"

// EventLoop
void ServerManager::run() {
	Demultiplexer	reactor(serverFds_);
	EventHandler 	eventHandler();
	TimeoutHandler	timeoutHandler();
	ClientManager	clientManager();

	while (isRunning()) { //g_signal 변수 명을 isRunning으로 둬도 될 듯
		int numEvents = reactor.waitForEvent();

		for (int i = 0; i < numEvents; ++i) {
			int type = reactor.getEventType(i);
			int fd = reactor.getSocketFd(i);

			if (type == EXCEPTION_EVENT) {
				// error response 전송 여부 판단 후 추가
				removeClientInfo(fd, clientManager, reactor, timeoutHandler);
			} else if (type == READ_EVENT) {
				if (isServer(fd)) {
					int clientFd = eventHandler.handleServerReadEvent(clientManager);
					if (clientFd) {
						reactor.addSocket(fd);
						timeoutHandler.addConnection(clientFd);
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

}

void ServerManager::cleanUpConnections(ClientManager& clientManager, eventHandler& eventHandler) {
	//clientManager의 ClientSessionMapIter => 연결되어있는 client에 eventHandler.handleShutDown(clientSession);
	//후에 clientManager.removeClient(fd);
}

// ServerManager의 멤버 함수로 둘지, 별도의 함수로 둘지
void ServerManager::addClientInfo(int clientFd, ClientManager& clientManager, Demultiplexer& reactor, TimeoutHandler& timeoutHandler) {
	clientManager.addClient(clientFd);
	timeoutHandler.addConnection(clientFd);
	reactor.addSocket(clientFd);
}

void ServerManager::removeClientInfo(int clientFd, ClientManager& clientManager, Demultiplexer& reactor, TimeoutHandler& timeoutHandler) {
	clientManager.removeClient(clientFd);
	timeoutHandler.removeConnection(clientFd);
	reactor.removeSocket(clientFd);
}
