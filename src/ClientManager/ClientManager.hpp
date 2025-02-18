#pragma once
#include <map>
#include <unistd.h>

//임시
class ClientSession {
	public:
		explicit ClientSession(int fd);
		~ClientSession();
		
};

class ClientManager {
	public:
		typedef std::map<int, ClientSession*>	TypeClientSessionMap;
		ClientManager();
		~ClientManager();
		void						addClient(int fd);
		ClientSessionMap::iterator	removeClient(int fd);
		ClientSession				accessClientSession(int fd);
		ClientSessionMap			accessClientSessionMap();

	private:
		TypeClientSessionMap	clientList_;

};
