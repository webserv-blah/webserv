#include "ServerManager.hpp"
#include "../demultiplexer/KqueueDemultiplexer.hpp"


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
				//error response 전송 여부
				timeoutHandler.removeConnection(fd);
				reactor.removeSocket(fd);
			} else if (type == READ_EVENT) {
				if (isServer(fd)) {
					int clientFd = eventHandler.handleServerReadEvent(clientManager);
					if (clientFd) { // if accepted
						reactor.addSocket(fd);
						timeoutHandler.addConnection(clientFd);
					}
				} else {
					int status = eventHandler.handleClientReadEvent();
					if (status == ONGOING) {
						reactor.addWriteEvent(fd);
					} else if (status == CONNECTION_CLOSED) {
						timeoutHandler.removeConnection(fd);
						reactor.removeSocket(fd);
					}
				}
			} else if (type == WRITE_EVENT) {
				int status = eventHandler.handleClientWriteEvent();
				if (status == DONE_WRITING) {
					reactor.removeWriteEvent(fd);
				} else if (status == CONNECTION_CLOSED) {
					timeoutHandler.removeConnection(fd);
					reactor.removeSocket(fd);
				}
			} 
		}

		timeoutHandler.checkTimeouts(eventHandler, reactor, clientManager); //ref로 넘김
	}

}
