#pragma once

#include "RequestMessage.hpp"

#define ONELINE_MAX_LENGTH	8190//8KB - 2bytes(\r\n)
#define URI_MAX_LENGTH		2048//2KB

class RequestParser {
	public:
		RequestParser();
		~RequestParser();

		std::string		parse(const std::string &readData, RequestMessage &reqMsg);
		//void			setBodyMaxLength(ssize_t length);

	private:
		ssize_t			oneLineMaxLength_;
		ssize_t			uriMaxLength_;
		//Optional<ssize_t>	bodyMaxLength_;//Client마다 바뀌는 설정 값
		
		void			handleOneLine(const std::string &line, RequestMessage &reqMsg);
		TypeReqStatus	setStatusCRLF(const TypeReqStatus &curStatus);
		void			parseStartLine(const std::string &line, RequestMessage &reqMsg);
		void			parseFieldLine(const std::string &line, RequestMessage &reqMsg);
		void			parseBody(const std::string &line, RequestMessage &reqMsg);
		//void				cleanUpChunkedBody(const std::string &line);
};

