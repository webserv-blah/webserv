#ifndef CLIENT_SESSION_HPP
#define CLIENT_SESSION_HPP

#include "CgiProcessInfo.hpp"
#include "../RequestMessage/RequestMessage.hpp"
#include "../GlobalConfig/GlobalConfig.hpp"
#include "../include/commonEnums.hpp"

class ClientSession {
	public:
		ClientSession(int listenFd, int clientFd, std::string clientIP);
		~ClientSession();

		int						getListenFd() const;
		int						getClientFd() const;
		int						getErrorStatusCode() const;
		const RequestMessage	*getReqMsg() const;
		const RequestConfig		*getConfig() const;
		std::string				getReadBuffer() const;
		std::string				getWriteBuffer() const;
		std::string				getClientIP() const;
		const CgiProcessInfo	*getCgiProcessInfo() const;

		void					setListenFd(const int &listenFd);
		void					setClientFd(const int &clientFd);
		void					setErrorStatusCode(const int &statusCode);
		void					setReqMsg(RequestMessage *reqMsg);
		void					setConfig(const RequestConfig *config);
		void					setReadBuffer(const std::string &remainData);
		void					setWriteBuffer(const std::string &remainData);
		void					setCgiProcessInfo(CgiProcessInfo &cgiProcessInfo);
		bool					isReceiving() const;

		RequestMessage			&accessReqMsg();
		std::string				&accessReadBuffer();
		CgiProcessInfo			&accessCgiProcessInfo();
		
	private:
		int						listenFd_;
		int						clientFd_;
		std::string				clientIP_;

		int						errorStatusCode_;
	
		RequestMessage			*reqMsg_;
	
		const RequestConfig		*config_;
		const RequestConfig		*defConfig_;
	
		std::string				readBuffer_;
		std::string				writeBuffer_;

		CgiProcessInfo			cgiProcessInfo_;

		void					resetRequest();
};

#endif
