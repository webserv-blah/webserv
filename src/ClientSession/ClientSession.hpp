#ifndef CLIENT_SESSION_HPP
#define CLIENT_SESSION_HPP

#include "commonEnums.hpp"
#include "GlobalConfig.hpp"
#include "RequestMessage.hpp"

class ClientSession {
	public:
		ClientSession(int listenFd, int clientFd, std::string clientIP);
		~ClientSession();

		int						getListenFd() const;
		int						getClientFd() const;
		int						getErrorStatusCode() const;
		EnumSesStatus			getStatus() const;
		const RequestMessage	*getReqMsg() const;
		const RequestConfig		*getConfig() const;
		std::string				getReadBuffer() const;
		size_t					getReadCursor() const;
		std::string				getWriteBuffer() const;
		std::string				getClientIP() const;

		void					setListenFd(const int &listenFd);
		void					setClientFd(const int &clientFd);
		void					setErrorStatusCode(const int &statusCode);
		void					setStatus(const EnumSesStatus &status);
		void					setReadBuffer(const std::string &remainData);
		void					setReadCursor(const size_t &curCursor);
		void					setWriteBuffer(const std::string &remainData);

		RequestMessage			&accessReqMsg();
		std::string				&accessReadBuffer();

	private:
		int						listenFd_;
		int						clientFd_;
		int						errorStatusCode_;
		EnumSesStatus			status_;
		RequestMessage			*reqMsg_;
		const RequestConfig		*config_;
		std::string				readBuffer_;
		size_t					readCursor_;
		std::string				writeBuffer_;
		std::string				clientIP_;
};

#endif
