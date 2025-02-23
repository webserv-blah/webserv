#pragma once

#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstdio>

//임시 인클루드
#include "../include/commonEnums.hpp"
#include "../ClientManager/ClientManager.hpp"
#include "../RequestMessage/RequestMessage.hpp"
//임시 클래스
class RequestParser {};
class StaticHandler { 
	public:
		StaticHandler(ResponseBuilder& rspBuilder_);
		~StaticHandler();
};
class CgiHandler {};
class ResponseBuilder {};

class EventHandler {
	public:
		EventHandler();
		~EventHandler();
		int		handleServerReadEvent(int fd);
		int		handleClientReadEvent(ClientSession& clientSession);
		int 	handleClientWriteEvent(ClientSession& clientSession);
		void	handleError(int statusCode, ClientSession& clientSession);
		
	private:
		RequestParser	parser_;
		ResponseBuilder	rspBuilder_;
		StaticHandler	staticHandler_;
		CgiHandler		cgiHandler_;

		EnumSesStatus		recvRequest(ClientSession& clientSession); //recv()
		EnumSesStatus		sendResponse(ClientSession& clientSession); //send()

};