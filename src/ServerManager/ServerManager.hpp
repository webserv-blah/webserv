#pragma once
#include <set>
#include "../Demultiplexer/KqueueDemultiplexer.hpp"
#include "../TimeoutHandler/TimeoutHandler.hpp"
#include "../EventHandler/EventHandler.hpp"
#include "../ClientManager/ClientManager.hpp"
#include "../include/commonEnums.hpp"

class ServerManager {
	public:
		ServerManager();
		~ServerManager();

		void	run();
		bool	isServer(int fd);
		bool	isRunning();

	private:
		std::set<int>	serverFds_;
		bool			serverStatus_; //signal 핸들링과 관련 있으므로 논의 필요

		void addClientInfo(int clientFd, ClientManager& clientManager, Demultiplexer& reactor, & timeoutHandler);
		void removeClientInfo(int clientFd, ClientManager& clientManager, Demultiplexer& reactor, & timeoutHandler);

};
