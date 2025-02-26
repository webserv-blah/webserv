#ifndef CLIENT_SESSION_HPP
#define CLIENT_SESSION_HPP

#include "../RequestParser/RequestParser.hpp"
#include "../GlobalConfig/GlobalConfig.hpp"
#include "../include/commonEnums.hpp"

class ClientSession {
	public:
		ClientSession(int listenFd, int clientFd, std::string clientIP);
		~ClientSession();

		int						getListenFd() const;
		int						getClientFd() const;
		int						getErrorStatusCode() const;
		const RequestMessage	&getReqMsg() const;
		const RequestConfig		&getConfig() const;
		const RequestMessage	*getReqMsgPtr() const;
		const RequestConfig		*getConfigPtr() const;
		std::string				getReadBuffer() const;
		std::string				getWriteBuffer() const;
		std::string				getClientIP() const;
		void					setListenFd(const int &listenFd);
		void					setClientFd(const int &clientFd);
		void					setErrorStatusCode(const int &statusCode);
		void					setReqMsg(RequestMessage *reqMsg);
		void					setConfig(const RequestConfig *config);
		void					setReadBuffer(const std::string &remainData);
		void					setWriteBuffer(const std::string &remainData);

		RequestMessage			&accessReqMsg();
		std::string				&accessReadBuffer();
		void					resetRequest();
		
	private:
		int						listenFd_;
		int						clientFd_;
		int						errorStatusCode_;
		RequestMessage			*reqMsg_;
		const RequestConfig		*config_;
		std::string				readBuffer_;
		std::string				writeBuffer_;
		std::string				clientIP_;
};

#endif
