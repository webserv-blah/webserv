#ifndef CLIENT_MANAGER_HPP
#define CLIENT_MANAGER_HPP

#include "./ClientSession.hpp"
#include <map>
#include <unistd.h>

class ClientManager {
	public:
		typedef std::map<int, ClientSession*>	TypeClientMap;
		ClientManager();
		~ClientManager();
		void					addClient(int listenFd, int clientFd);
		TypeClientMap::iterator	removeClient(int fd);
		ClientSession*			accessClientSession(int fd);
		TypeClientMap&			accessClientSessionMap();

	private:
		TypeClientMap	clientList_;

};


#endif
