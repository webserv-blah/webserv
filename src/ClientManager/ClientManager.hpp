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
		typedef std::map<int, ClientSession*>	ClientSessionMap;
		ClientManager();
		~ClientManager();
		void			addClient(int fd);
		void			removeClient(int fd);
		ClientSession	accessClientSession(int fd);

	private:
		ClientSessionMap	clientList_;

};
