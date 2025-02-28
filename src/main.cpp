#include <iostream>
#include "utils/utils.hpp"
#include "GlobalConfig/GlobalConfig.hpp"
#include "ServerManager/ServerManager.hpp"

volatile bool globalServerRunning = true;

int main(int argc, char** argv) {
    if (argc != 2) {
		std::cerr << "Usage: ./webserver [config file path]" << std::endl;
		return 1;
	}
	try {
		std::clog << "Setting up Webserv..." << std::endl;
		GlobalConfig::initGlobalConfig(argv[1]);
		setupSignalHandlers();
		ServerManager serverManager;
		serverManager.setupListeningSockets();
		serverManager.run();
	} catch (const std::exception& e) {
		GlobalConfig::destroyInstance();
		if (globalServerRunning) {
			std::cerr << "Error: " << e.what() << std::endl;
		}
		return 1;
	}
	GlobalConfig::destroyInstance();
	return 0;
}
