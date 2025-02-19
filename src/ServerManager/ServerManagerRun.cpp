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

			if (type == TypeEvent::EXCEPTION_EVENT) {
				// error response 전송 여부 판단 후 추가
				removeClientInfo(fd, clientManager, reactor, timeoutHandler);
			} else if (type == TypeEvent::READ_EVENT) {
				if (isServer(fd)) {
					int clientFd = eventHandler.handleServerReadEvent(fd);

					if (clientFd) {
						addClientInfo(clientFd, clientManager, reactor, timeoutHandler);
					}

				} else {
					TypeSesStatus	status = eventHandler.handleClientReadEvent(clientManager.accessClientSession(fd));

					if (status == TypeSesStatus::CONNECTION_CLOSED) {
						removeClientInfo(fd, clientManager, reactor, timeoutHandler);
					} else if (status == TypeSesStatus::CONNECTION_ERROR) {
						// connection error 처리 로직 추가
					} else if (status == TypeSesStatus::WRITE_CONTINUE) {
						timeoutHandler.updateActivity(fd);
						reactor.addWriteEvent(fd);
					} else {
						timeoutHandler.updateActivity(fd);
					}
				}

			} else if (type == TypeEvent::WRITE_EVENT) {
				TypeSesStatus	status = eventHandler.handleClientWriteEvent(clientManager.accessClientSession(fd));

				if (status == TypeSesStatus::CONNECTION_CLOSED) {
					removeClientInfo(fd, clientManager, reactor, timeoutHandler);
				} else if (status == TypeSesStatus::WRITE_COMPLETE) {
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

// ServerManager의 멤버 함수로 둘지, 별도의 함수로 둘지
void ServerManager::addClientInfo(int clientFd, ClientManager& clientManager, Demultiplexer& reactor, & timeoutHandler) {
	clientManager.addClient(clientFd);
	timeoutHandler.addConnection(clientFd);
	reactor.addSocket(clientFd);
}

void ServerManager::removeClientInfo(int clientFd, ClientManager& clientManager, Demultiplexer& reactor, & timeoutHandler) {
	clientManager.removeClient(clientFd);
	timeoutHandler.removeConnection(clientFd);
	reactor.removeSocket(clientFd);
}

