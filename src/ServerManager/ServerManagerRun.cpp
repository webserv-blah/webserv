#include "ServerManager.hpp"

// EventLoop 실행 함수
void ServerManager::run() {
	Demultiplexer	reactor(listenFds_); // I/O 멀티플렉싱을 위한 리액터 객체
	EventHandler 	eventHandler; // 이벤트 처리 담당 객체
	TimeoutHandler	timeoutHandler; // 클라이언트 타임아웃 관리 객체
	ClientManager	clientManager; // 클라이언트 세션 관리 객체

	while (isServerRunning()) { // isServerRunning() 함수로 서버 실행 여부 확인 (전역변수 플래그 대체 가능)
		int	numEvents = reactor.waitForEvent(); // 발생한 이벤트 개수 확인

		for (int i = 0; i < numEvents; ++i) {
			EnumEvent	type = reactor.getEventType(i); // 이벤트 타입 확인
			int 		fd = reactor.getSocketFd(i); // 이벤트가 발생한 소켓 FD 확인

			if (type == EXCEPTION_EVENT) { // 예외(오류) 이벤트 발생 시
				removeClientInfo(fd, clientManager, reactor, timeoutHandler); // 클라이언트 제거
			} else if (type == READ_EVENT) { // 읽기(수신) 이벤트 발생 시
				if (isListeningSocket(fd)) { // 서버 소켓이라면 새 클라이언트 연결 처리
					int clientFd = eventHandler.handleServerReadEvent(fd);

					if (clientFd > 0) { // 새로운 클라이언트가 정상적으로 연결됨
						addClientInfo(fd, clientFd, clientManager, reactor, timeoutHandler);
					}

				} else { // 클라이언트 소켓에서 데이터 수신 처리
					ClientSession*	client = clientManager.accessClientSession(fd);
					if (!client) {
						perror("Invalid Client Fd");
						continue ;
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

			} else if (type == WRITE_EVENT) { // 쓰기(송신) 이벤트 발생 시
				ClientSession*	client = clientManager.accessClientSession(fd);
				if (!client) {
					perror("Invalid Client Fd");
					continue ;
				}

				EnumSesStatus	status = eventHandler.handleClientWriteEvent(*client);
				if (status == CONNECTION_CLOSED) { // 클라이언트가 연결 종료
					removeClientInfo(fd, clientManager, reactor, timeoutHandler);
				} else if (status == WRITE_COMPLETE) { // 데이터 전송 완료
					timeoutHandler.updateActivity(fd); // 타임아웃 갱신((지울지 여부 판단 필요))
					reactor.removeWriteEvent(fd); // 쓰기 이벤트 제거
				} else { // 쓰기 작업 진행 중 -> 타임아웃 갱신
					timeoutHandler.updateActivity(fd); // 타임아웃 갱신((지울지 여부 판단 필요))
				}

			}
		}
		// 타임아웃된 클라이언트 처리
		timeoutHandler.checkTimeouts(eventHandler, reactor, clientManager);
	}

	// 서버 종료 시, 연결된 클라이언트 정리
	cleanUpConnections(clientManager, eventHandler);
}

// 서버 종료 전, 모든 클라이언트에 종료 응답을 전송하고 연결된 리소스를 정리하는 함수
void ServerManager::cleanUpConnections(ClientManager& clientManager, EventHandler& eventHandler) {
	std::map<int, ClientSession*>&			clientList = clientManager.accessClientSessionMap();
	std::map<int, ClientSession*>::iterator	it;

	for (it = clientList.begin(); it != clientList.end(); ) {
		eventHandler.handleError(503, *it->second); // 클라이언트에게 서버 종료 알림
		it = clientManager.removeClient(it->first); // 클라이언트 세션 삭제
	}
}

// ((ServerManager 멤버 함수로 둘지, 별도의 함수로 만들지 고민 필요))
// 새로운 클라이언트 정보 추가 함수
void ServerManager::addClientInfo(int listenFd, int clientFd, ClientManager& clientManager, Demultiplexer& reactor, TimeoutHandler& timeoutHandler) {
	clientManager.addClient(listenFd, clientFd); // 클라이언트 추가
	timeoutHandler.addConnection(clientFd); // 타임아웃 관리 추가
	reactor.addSocket(clientFd); // 리액터(이벤트 루프)에 소켓 등록
}

// 기존 클라이언트 정보 삭제 함수
void ServerManager::removeClientInfo(int clientFd, ClientManager& clientManager, Demultiplexer& reactor, TimeoutHandler& timeoutHandler) {
	clientManager.removeClient(clientFd); // 클라이언트 제거
	timeoutHandler.removeConnection(clientFd); // 타임아웃 관리 제거
	reactor.removeSocket(clientFd); // 리액터에서 소켓 제거
}
