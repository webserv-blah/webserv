#ifndef EVENT_HANDLER_HPP
#define EVENT_HANDLER_HPP

#include "../include/commonEnums.hpp"
#include "../ClientManager/ClientManager.hpp"
#include "../RequestMessage/RequestMessage.hpp"
#include "../RequestParser/RequestParser.hpp"
#include "../RequestHandler/StaticHandler.hpp"
#include "../RequestHandler/CgiHandler.hpp"
#include "../ResponseBuilder/ResponseBuilder.hpp"

// EventHandler 객체는 ServerManager::run()에서 사용됩니다.
// ../ServerManager/ServerManagerRun.cpp
// 이벤트 처리를 담당하는 클래스
class EventHandler {
	public:
		EventHandler();
		~EventHandler();
		int				handleServerReadEvent(int fd, ClientManager& clientManager);
		EnumSesStatus	handleClientReadEvent(ClientSession& clientSession);
		EnumSesStatus 	handleClientWriteEvent(ClientSession& clientSession);
		void			handleError(int statusCode, ClientSession& clientSession);
		
	private:
		RequestParser	parser_;
		ResponseBuilder	rspBuilder_;
		StaticHandler	staticHandler_;
		CgiHandler		cgiHandler_;

		EnumSesStatus	recvRequest(ClientSession& clientSession); //recv()
		EnumSesStatus	sendResponse(ClientSession& clientSession); //send()

};

#endif
