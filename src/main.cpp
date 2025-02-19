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
