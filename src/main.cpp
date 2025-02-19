#include <iostream>
#include "GlobalConfig.hpp"
#include "ClientSession.hpp"
EnumSesStatus readRequest(ClientSession &curSession, RequestParser &parser);
// #include "ServerManager.hpp"

int main(int argc, char** argv) {
	/*
    GlobalConfig globalConfig;

    ConfigParser::parse(globalConfig, argv[1]);
    globalConfig.print();
    return 0;
	*/
	ClientSession ses(0);
	RequestParser parser;
	readRequest(ses, parser);
}

// #include <iostream>
// #include "utils.hpp"
// #include "GlobalConfig.hpp"
// #include "ServerManager.hpp"

// volatile bool globalServerRunning = true;

// int main(int argc, char** argv) {
//     if (argc != 2) {
// 		std::cerr << "Usage: ./webserver [config file path]" << std::endl;
// 		return 1;
// 	}
// 	GlobalConfig::initGlobalConfig(argv[1]);
// 	GlobalConfig::getInstance().print();
// 	setupSignalHandlers();
// 	ServerManager serverManager;
// 	serverManager.setupListeningSockets();
// 	serverManager.print();
// 	// serverManager.run();
// 	return 0;
// }