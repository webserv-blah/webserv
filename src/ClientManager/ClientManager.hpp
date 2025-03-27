#ifndef CLIENT_MANAGER_HPP
#define CLIENT_MANAGER_HPP

#include "../ClientSession/ClientSession.hpp"
#include <map>
#include <unistd.h>

class ClientManager {
	public:
		typedef std::map<int, ClientSession*>	TypeClientMap;
		std::map<int, int>						pipeToClientFdMap_;
		ClientManager();
		~ClientManager();
		void					addClient(int listenFd, int clientFd, std::string clientAddr);
		void					addPipeMap(int outPipe, int clientFd);
		
		void					removePipeFromMap(int pipeFd);
		TypeClientMap::iterator	removeClient(int fd);
		ClientSession*			accessClientSession(int fd);
		TypeClientMap&			accessClientSessionMap();
		int						accessClientFd(int pipeFd);

		bool					isClientSocket(int fd);

	private:
		TypeClientMap	clientList_;

};

#endif
