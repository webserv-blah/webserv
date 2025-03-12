#include "ClientManager.hpp"
#include "../include/errorUtils.hpp"

ClientManager::ClientManager() {
	// 생성자
}

ClientManager::~ClientManager() {
	// 소멸자: 모든 클라이언트 세션을 정리
	TypeClientMap::iterator it;
	for (it = clientList_.begin(); it != clientList_.end(); ++it) {
		delete it->second; // 동적 할당된 ClientSession 객체 해제
		close(it->first);  // 해당 클라이언트의 소켓 닫기
	}
}

// 새로운 clientSession 생성 및 관리 목록에 추가
void ClientManager::addClient(int listenFd, int clientFd, std::string clientAddr) {
	// 중복 검사 필요 여부 고려
	ClientSession* newClient = new ClientSession(listenFd, clientFd, clientAddr);
	clientList_.insert(std::make_pair(clientFd, newClient)); // clientFd를 key로 하여 추가
}

void	ClientManager::removePipeFromMap(int pipeFd) {
	std::map<int, int>::iterator it = pipeToClientFdMap_.find(pipeFd);
	if (it == pipeToClientFdMap_.end()) {
		// 존재하지 않는 fd에 대한 요청 시 오류 로깅
		webserv::logError(WARNING, "Pipe Fd Not Found", 
		                 "fd: " + utils::size_t_tos(pipeFd), 
		                 "ClientManager::removePipeToClientMap");
		return;
	}
	pipeToClientFdMap_.erase(it);
}

// clientSession 제거 및 목록에서 삭제
ClientManager::TypeClientMap::iterator ClientManager::removeClient(int fd) {
	TypeClientMap::iterator it = clientList_.find(fd);
	if (it == clientList_.end()) {
		// 존재하지 않는 fd에 대한 요청 시 오류 로깅
		webserv::logError(WARNING, "Client Fd Not Found", 
		                 "fd: " + utils::size_t_tos(fd), 
		                 "ClientManager::removeClient");
		return clientList_.end(); // 유효하지 않은 반복자 반환
	}

	delete it->second; // 동적으로 할당된 ClientSession 해제
	close(fd); // 해당 클라이언트의 소켓 닫기

	// 해당 요소를 map에서 삭제하고 다음 요소의 반복자 반환
	TypeClientMap::iterator nextIt = it;
	++nextIt;
	clientList_.erase(it);
	return nextIt;
}

// fd에 대응하는 clientSession 객체 반환
ClientSession* ClientManager::accessClientSession(int fd) {
	// 해당 fd가 존재하지 않으면 NULL 반환
	TypeClientMap::iterator it = clientList_.find(fd);
	if (it == clientList_.end()) {
		return NULL;
	}
	return it->second; // ClientSession 포인터 반환
}

// 전체 clientSession 목록(clientList_) 참조 반환
ClientManager::TypeClientMap& ClientManager::accessClientSessionMap() {
	return clientList_;
}

bool	ClientManager::isClientSocket(int fd) {
	// 해당 fd가 존재하지 않으면 NULL 반환
	TypeClientMap::iterator it = clientList_.find(fd);
	if (it == clientList_.end()) {
		return false;
	}
	return true; // ClientSession 포인터 반환
}