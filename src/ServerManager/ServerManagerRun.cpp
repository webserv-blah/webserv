#include "ServerManager.hpp"
#include "CgiProcessInfo.hpp"
#include "../include/errorUtils.hpp"
#include <unistd.h>

// 서버의 메인 이벤트 루프를 실행합니다.
// - EventHandler: 클라이언트 및 서버 이벤트 처리를 담당합니다.
// - ClientManager: 클라이언트 세션을 관리합니다.
// - Demultiplexer: I/O 멀티플렉싱을 통해 여러 소켓의 이벤트를 감지합니다.
// - TimeoutHandler: 클라이언트 연결의 타임아웃을 관리합니다.
//
// 서버가 실행되는 동안 발생하는 이벤트(EXCEPTION, READ, WRITE)를
// 감지하여 각각의 처리 함수를 호출합니다.
void ServerManager::run() {
	EventHandler					eventHandler;
	ClientManager					clientManager;
	Demultiplexer					reactor(listenFds_);
	TimeoutHandler					timeoutHandler;
	std::map<int, int>				pipeToClientSessionMap;

	while (isServerRunning()) {
		// 발생한 이벤트의 개수를 확인
		DEBUG_LOG("[ServerManager]Waiting for events...")
		timespec* timeout = timeoutHandler.getEarliestTimeout();
		int	numEvents = reactor.waitForEvent(timeout);

		// 발생한 각 이벤트를 순회하며 처리
		for (int i = 0; i < numEvents; ++i) {
			EnumEvent	type = reactor.getEventType(i);
			int 		fd = reactor.getSocketFd(i);

			if (type == READ_EVENT) {
				if (isListeningSocket(fd)) {
					DEBUG_LOG("[ServerManager]READ Event on Listening Socket: fd " << fd)
					// 리스닝 소켓에서 읽기 이벤트 발생: 새로운 클라이언트의 연결 요청 처리
					processServerReadEvent(
						fd, clientManager, eventHandler, timeoutHandler, reactor);
				} else if (clientManager.isClientSocket(fd)) {
					DEBUG_LOG("[ServerManager]READ Event on Client Socket: fd " << fd)
					// 기존 클라이언트 소켓에서 읽기 이벤트 발생: 클라이언트로부터 데이터 수신 처리
					processClientReadEvent(
						fd, clientManager, eventHandler, timeoutHandler, reactor);
				} else {
					processCgiReadEvent(fd, clientManager, eventHandler, timeoutHandler, reactor);
				}
			} else if (type == WRITE_EVENT) {
				DEBUG_LOG("[ServerManager]WRITE Event on Client Socket: fd " << fd)
				// 쓰기 이벤트 발생: 클라이언트에게 데이터를 전송하는 작업 처리
				processClientWriteEvent(
					fd, clientManager, eventHandler, timeoutHandler, reactor);
			}
		}
		// 타임아웃된 클라이언트 확인 후 처리
		timeoutHandler.checkTimeouts(eventHandler, reactor, clientManager);
	}
}

// 새로운 클라이언트가 연결되었을 때 호출됩니다.
void ServerManager::addClientInfo(int clientFd, Demultiplexer& reactor, TimeoutHandler& timeoutHandler) {
	// 클라이언트의 타임아웃 관리를 시작
	timeoutHandler.addConnection(clientFd);
	// 리액터에 클라이언트 소켓을 추가하여 이벤트 감시 대상에 포함
	reactor.addReadEvent(clientFd);
}

// 클라이언트 연결 종료 또는 오류 발생 시 호출됩니다.
void ServerManager::removeClientInfo(int clientFd, ClientManager& clientManager, Demultiplexer& reactor, TimeoutHandler& timeoutHandler) {
	// 클라이언트 세션을 ClientManager에서 제거
	clientManager.removeClient(clientFd);
	// 타임아웃 관리 대상에서 클라이언트 삭제
	timeoutHandler.removeConnection(clientFd);
	// 리액터에서 클라이언트 소켓 제거
	reactor.removeFd(clientFd);
	DEBUG_LOG("[ServerManager]Removed Client Socket " << clientFd)
}

// 리스닝 소켓에서 읽기 이벤트가 발생하면 새로운 클라이언트의 연결 요청을 처리합니다.
// handleServerReadEvent()가 새 클라이언트 연결을 수락하고, 새 클라이언트 FD를 반환합니다.
// 반환된 FD가 유효하면 addClientInfo()를 통해 클라이언트 정보를 등록합니다.
void ServerManager::processServerReadEvent(int fd, ClientManager& clientManager,
	EventHandler& eventHandler, TimeoutHandler& timeoutHandler, Demultiplexer& reactor) {
	// 새로운 클라이언트 연결 요청 처리 및 소켓 FD 반환
	int clientFd = eventHandler.handleServerReadEvent(fd, clientManager);

	// 새로 연결된 클라이언트의 소켓 FD가 유효한 경우 클라이언트 정보 등록
	if (clientFd > 0) {
		addClientInfo(clientFd, reactor, timeoutHandler);
	}
}

// 기존 클라이언트 소켓에서 읽기 이벤트가 발생한 경우, 클라이언트 데이터를 처리합니다.
void ServerManager::processClientReadEvent(int fd, ClientManager& clientManager,
	EventHandler& eventHandler, TimeoutHandler& timeoutHandler, Demultiplexer& reactor) {
	// 파일 디스크립터에 해당하는 클라이언트 세션을 획득
	ClientSession* client = clientManager.accessClientSession(fd);
	if (!client) {
		// 유효하지 않은 클라이언트 FD인 경우 경고 로깅 후 종료
		webserv::logError(WARNING, "Invalid Value", 
		                 "No clientSession corresponding to fd: " + utils::size_t_tos(fd), 
		                 "ServerManager::processClientReadEvent");
		return;
	}

	// 클라이언트로부터 데이터를 읽어 처리한 후 반환 상태를 확인
	EnumSesStatus status = eventHandler.handleClientReadEvent(*client);
	if (status == CONNECTION_CLOSED) { 
		// 클라이언트가 연결을 종료한 경우, 관련 정보를 삭제
		removeClientInfo(fd, clientManager, reactor, timeoutHandler);
	} else if (status == WAIT_FOR_CGI) {
		timeoutHandler.updateActivity(fd, status);
		reactor.addReadEvent(client->getCgiProcessInfo()->outPipe_);
		clientManager.addPipeMap(client->getCgiProcessInfo()->outPipe_, client->getClientFd());
	} else if (status == WRITE_COMPLETE) {
		timeoutHandler.updateActivity(fd, status);
	} else if (status == WRITE_CONTINUE) {
		timeoutHandler.updateActivity(fd, status);
		reactor.addWriteEvent(fd);
	}
}

void	ServerManager::processCgiReadEvent(int pipeFd, ClientManager& clientManager,
	EventHandler& eventHandler, TimeoutHandler& timeoutHandler, Demultiplexer& reactor) {
	// 파일 디스크립터에 해당하는 클라이언트 세션을 획득
	int clientFd = clientManager.accessClientFd(pipeFd);
	if (clientFd == -1) {
		// 유효하지 않은 파이프 FD인 경우 경고 로깅 후 종료
		webserv::throwError("Invalid Value", 
		                 "No clientSession corresponding to pipeFd: " + utils::size_t_tos(pipeFd), 
		                 "ServerManager::processCgiReadEvent");
		return;
	}
	ClientSession* client = clientManager.accessClientSession(clientFd);
	if (!client) {
		// 유효하지 않은 클라이언트 FD인 경우 경고 로깅 후 종료
		webserv::throwError("Invalid Value", 
		                 "No clientSession corresponding to fd: " + utils::size_t_tos(clientFd), 
		                 "ServerManager::processCgiReadEvent");
		return;
	}
	// CGI로부터 데이터를 읽어 처리한 후 반환 상태를 확인
	EnumSesStatus status = eventHandler.handleCgiReadEvent(*client);

	 // 파이프 처리 결과에 따른 상태 처리
	 switch (status) {
        case WAIT_FOR_CGI:
            // CGI 작업이 진행 중이므로 계속 대기
            break;
            
        case CONNECTION_CLOSED:
            // 연결이 종료된 경우 클라이언트와 파이프 모두 정리
            removeClientInfo(clientFd, clientManager, reactor, timeoutHandler);
            break;
            
        case WRITE_COMPLETE:
            // 응답 준비 완료, 클라이언트 활성 시간 갱신
            timeoutHandler.updateActivity(clientFd, status);
            break;
            
        case WRITE_CONTINUE:
            // 클라이언트에 데이터 쓰기 준비
            timeoutHandler.updateActivity(clientFd, status);
            reactor.addWriteEvent(clientFd);
            break;
            
        default:
            // 예상치 못한 상태 처리
            webserv::throwError("Unexpected Status", 
                             "Unhandled status code: " + utils::size_t_tos(status), 
                             "ServerManager::processCgiReadEvent");
            break;
    }
    
    // WAIT_FOR_CGI 상태가 아닌 경우 파이프를 리액터에서 제거
    if (status != WAIT_FOR_CGI) {
		clientManager.removePipeFromMap(pipeFd);
        reactor.removeFd(pipeFd);
    }
}

// 클라이언트 소켓에 쓰기(전송) 이벤트가 발생한 경우 데이터를 전송합니다.
void ServerManager::processClientWriteEvent(int fd, ClientManager& clientManager,
	EventHandler& eventHandler, TimeoutHandler& timeoutHandler, Demultiplexer& reactor) {
	ClientSession* client = clientManager.accessClientSession(fd);
	if (!client) {
		// 유효하지 않은 클라이언트 FD인 경우 경고 로깅 후 종료
		webserv::logError(WARNING, "Invalid Value", 
		                 "No clientSession corresponding to fd: " + utils::size_t_tos(fd), 
		                 "ServerManager::processClientWriteEvent");
		return;
	}

	// 클라이언트에 데이터 전송 후 반환된 상태 확인
	EnumSesStatus status = eventHandler.handleClientWriteEvent(*client);
	if (status == CONNECTION_CLOSED) { 
		// 클라이언트가 연결을 종료한 경우, 관련 정보를 삭제
		removeClientInfo(fd, clientManager, reactor, timeoutHandler);
	} else if (status == WRITE_COMPLETE) { 
		// 데이터 전송이 완료된 경우, 리액터에서 쓰기 이벤트를 제거하여 더 이상 쓰기 이벤트를 감시하지 않음
		reactor.removeWriteEvent(fd);
	}
	// status == WRITE_CONTINUE인 경우 계속 쓰기 이벤트를 감시
}
