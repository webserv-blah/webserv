#include "ServerManager.hpp"
#include "../demultiplexer/KqueueDemultiplexer.hpp"

//Event Loop
// 읽기 혹은 쓰기 상태 관련 enum(혹은 매크로) 논의 필요
// 아래 사용된 status 매크로는 임시

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
				timeoutHandler.removeConnection(fd);
				reactor.removeSocket(fd);
			} else if (type == READ_EVENT) {
				if (isServer(fd)) {
					int clientFd = eventHandler.handleServerReadEvent(clientManager);
					if (clientFd) {
						reactor.addSocket(fd);
						timeoutHandler.addConnection(clientFd);
					}
				} else {
					int status = eventHandler.handleClientReadEvent();
					if (status == CONNECTION_CLOSED) {
						timeoutHandler.removeConnection(fd);
						reactor.removeSocket(fd);
					} else if (status == WRITE_ONGOING) {
						timeoutHandler.updateActivity(fd);
						reactor.addWriteEvent(fd);
					} else {
						timeoutHandler.updateActivity(fd);
					}
				}
			} else if (type == WRITE_EVENT) 
				int status = eventHandler.handleClientWriteEvent();
				if (status == CONNECTION_CLOSED) {
					timeoutHandler.removeConnection(fd);
					reactor.removeSocket(fd);
				} else if (status == DONE_WRITING) {
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
