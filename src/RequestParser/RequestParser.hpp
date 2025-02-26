#ifndef REQUEST_PARSER_HPP
#define REQUEST_PARSER_HPP

#include "../RequestMessage/RequestMessage.hpp"
#include "../include/commonEnums.hpp"

#define ONELINE_MAX_LENGTH	8190//8KB - 2bytes(\r\n)
#define URI_MAX_LENGTH		2048//2KB
#define BODY_MAX_LENGTH		1048576//1MB

class RequestParser {
	public:
		RequestParser();
		~RequestParser();

		EnumStatusCode		parse(const std::string &readData, std::string &readBuffer, RequestMessage &reqMsg);
		void				setConfigBodyLength(size_t length);

	private:
		size_t				oneLineMaxLength_;
		size_t				uriMaxLength_;
		size_t				bodyMaxLength_;//Client마다 바뀌는 설정 값
		
		EnumStatusCode		handleOneLine(const std::string &line, RequestMessage &reqMsg);
		EnumReqStatus		handleCRLFLine(const EnumReqStatus &curStatus);
		EnumStatusCode		parseStartLine(const std::string &line, RequestMessage &reqMsg);
		EnumStatusCode		parseFieldLine(const std::string &line, RequestMessage &reqMsg);
		bool				validateFieldValueCount(const std::string &name, const int count);
		bool				handleFieldValue(const std::string &name, const std::string &value, RequestMessage &reqMsg);
		EnumStatusCode		cleanUpChunkedBody(const std::string &data, std::string &readBuffer, RequestMessage &reqMsg);
		EnumStatusCode		parseBody(const std::string &line, std::string &readBuffer, RequestMessage &reqMsg);
};

#endif
