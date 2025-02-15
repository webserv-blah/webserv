#pragma once
#include <set>

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

};
