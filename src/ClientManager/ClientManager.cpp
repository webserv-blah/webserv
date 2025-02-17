#include "ClientManager.hpp"

ClientManager::ClientManager() {
	//map size 초기화 필요 여부 고민
}

ClientManager::~ClientManager() {

}

//clientSession 생성 및 관리 목록에 추가
void	ClientManager::addClient(int fd) {
	ClientSession* newClient = new ClientSession(fd);

	clientList_.insert(std::make_pair(fd, newClient));
}

//clientSession 제거 및 관리 목록에 추가
void	ClientManager::removeClient(int fd) {
	ClientSessionMap::iterator it = clientList_.find(fd);
	if (it == clientList_.end()) {
		perror("Client Fd Not Found");
		return ;
	}
	delete it->second;
	close(fd); //여기서 close 할지 밖에서 할지, clientSession에서 할지..?
	clientList_.erase(it);
}

//fd에 대응하는 clientSession 객체 반환
ClientSession	ClientManager::accessClientSession(int fd) {
	if (clientList_.find(fd) == clientList_.end()) {
		return (ClientSession)0;
	}
	return *clientList_[fd];
}
