#pragma once

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstdio>

//임시
#include "../include/commonEnums.hpp"
#include "../ClientManager/ClientManager.hpp"
class RequestParser {};
class StaticHandler {};
class CgiHandler {};
class ResponseBuilder {};

class EventHandler {
	public:
		EventHandler();
		~EventHandler();
		int		handleServerReadEvent(int fd);
		int		handleClientReadEvent(ClientSession& clientSession);
		int 	handleClientWriteEvent(ClientSession& clientSession);
		void	handleExceptionEvent(ClientSession& clientSession);
		void	handleTimeout(ClientSession& clientSession);
		void	handleServerShutDown(ClientSession& clientSession);
		
	private:
		RequestParser	parser_;
		StaticHandler	staticHandler_;
		CgiHandler		cgiHandler_;
		ResponseBuilder	rspBuilder_;

		int		readRequest(ClientSession& clientSession); //rcv()
		int		sendResponse(ClientSession& clientSession); //send()
		int		sendErrorResponse(ClientSession& clientSession); 

};