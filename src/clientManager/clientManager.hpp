#pragma once

#include "RequestManager.hpp"
#include <string>

class ClientManager {
	public:
		ClientManager(int fd);
		~ClientManager();

		std::string		readRequest();
		/*type*/		writeResponse();
	private:
		int				clientFd_;
		RequestManager	reqManager_;
		std::string		readBuffer_;
		std::string		writeBuffer_;
};
