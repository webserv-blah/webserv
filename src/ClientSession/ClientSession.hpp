#pragma once

#include "RequestParser.hpp"
#include "GlobalConfig.hpp"
#include "commonEnums.hpp"

class ClientSession {
	public:
		ClientSession(int listenFd, int clientFd);
		~ClientSession();

		int						getListenFd() const;
		int						getClientFd() const;
		EnumSesStatus			getStatus() const;
		std::string				getReadBuffer() const;
		std::string				getWriteBuffer() const;
		const RequestMessage	*getReqMsg() const;
		const RequestConfig		*getConfig() const;
		void					setListenFd(const int &listenFd);
		void					setClientFd(const int &clientFd);
		void					setStatus(const EnumSesStatus &status);
		void					setReadBuffer(const std::string &remainData);
		void					setWriteBuffer(const std::string &remainData);
		
		//Optional<ssize_t>	getConfBodyMax() const;
		EnumSesStatus			implementReqMsg(RequestParser &parser, const std::string &readData);

	private:
		int						listenFd_;
		int						errorStatusCode_;
		int						clientFd_;
		EnumSesStatus			status_;
		RequestMessage			*reqMsg_;
		RequestConfig			*config_;
		std::string				readBuffer_;
		std::string				writeBuffer_;
};
