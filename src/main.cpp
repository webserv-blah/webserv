#include <iostream>
#include "utils/utils.hpp"
#include "GlobalConfig/GlobalConfig.hpp"
#include "ServerManager/ServerManager.hpp"
#include "errorUtils.hpp"

volatile bool globalServerRunning = true;

int main(int argc, char** argv) {
    if (argc != 2) {
		std::cerr << "Usage: ./webserver [config file path]" << std::endl;
		return 1;
	}
	try {
		DEBUG_LOG("[main]Initializing global configuration...")
		GlobalConfig::initGlobalConfig(argv[1]);
		#ifdef DEBUG
		GlobalConfig::getInstance().print();
		#endif
		setupSignalHandlers();
		ServerManager serverManager;
		DEBUG_LOG("[main]Setting up listening sockets...")
		serverManager.setupListeningSockets();
		DEBUG_LOG("[main]Starting server manager...")
		serverManager.run();
	} catch (const std::exception& e) {
		GlobalConfig::destroyInstance();
		if (globalServerRunning) {
			std::cerr << "Error: " << e.what() << std::endl;
		}
		return 1;
	}
	DEBUG_LOG("[main]Server stopped successfully.")
	GlobalConfig::destroyInstance();
	return 0;
}
