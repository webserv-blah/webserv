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
		EnumSesStatus			getStatus() const;
		std::string				getClientIP() const;
		std::string				getReadBuffer() const;
		std::string				getWriteBuffer() const;
		const RequestMessage	&getReqMsg() const;
		const RequestConfig		&getConfig() const;
		void					setListenFd(const int &listenFd);
		void					setClientFd(const int &clientFd);
		void					setStatus(const EnumSesStatus &status);
		void					setReadBuffer(const std::string &remainData);
		void					setWriteBuffer(const std::string &remainData);
		
		EnumSesStatus			implementReqMsg(RequestParser &parser, const std::string &readData);

	private:
		int						listenFd_;
		int						errorStatusCode_;
		int						clientFd_;
		EnumSesStatus			status_;
		RequestMessage			*reqMsg_;
		const RequestConfig		*config_;
		std::string				readBuffer_;
		std::string				writeBuffer_;
		std::string				clientIP_;
};

#endif
