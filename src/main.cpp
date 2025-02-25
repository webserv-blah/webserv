#include <iostream>
#include "utils.hpp"
#include "GlobalConfig.hpp"
#include "ServerManager.hpp"

volatile bool globalServerRunning = true;

int main(int argc, char** argv) {
    if (argc != 2) {
		std::cerr << "Usage: ./webserver [config file path]" << std::endl;
		return 1;
	}
	try {
		GlobalConfig::initGlobalConfig(argv[1]);
		setupSignalHandlers();
		ServerManager serverManager;
		serverManager.setupListeningSockets();
		serverManager.run();
	} catch (const std::exception& e) {
		GlobalConfig::destroyInstance();
		std::cerr << "Error: " << e.what() << std::endl;
		return 1;
	}
	GlobalConfig::destroyInstance();
	return 0;
}
