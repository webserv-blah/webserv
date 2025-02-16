#pragma once

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstdio>

//임시
class RequestParser {};
class IRequestHandler {};
class ResponseBuilder {};

class EventHandler {
	public:
		EventHandler();
		~EventHandler();
		int		handleServerReadEvent(int fd);
		int		handleClientReadEvent(int fd);
		int 	handleClientWriteEvent(int fd);
		void	handleExceptionEvent(int fd);
		void	handleTimeout(int fd);
		void	handleServerShutDown(int fd);
		
	private:
		RequestParser	parser_;
		IRequestHandler	handler_;
		ResponseBuilder	rspBuilder_;

		int		readRequest(); //rcv()
		int		sendResponse(); //send()

};