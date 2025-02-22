#ifndef REQUEST_PARSER_HPP
# define REQUEST_PARSER_HPP

# include "RequestMessage.hpp"

# define ONELINE_MAX_LENGTH	8190//8KB - 2bytes(\r\n)
# define URI_MAX_LENGTH		2048//2KB

class RequestParser {
	public:
		RequestParser();
		~RequestParser();

		std::string		parse(const std::string &readData, RequestMessage &reqMsg);
		//void			setBodyMaxLength(size_t length);

	private:
		size_t			oneLineMaxLength_;
		size_t			uriMaxLength_;
		//Optional<size_t>	bodyMaxLength_;//Client마다 바뀌는 설정 값
		
		void			handleOneLine(const std::string &line, RequestMessage &reqMsg);
		EnumReqStatus	handleCRLFLine(const EnumReqStatus &curStatus);
		void			parseStartLine(const std::string &line, RequestMessage &reqMsg);
		void			parseFieldLine(const std::string &line, RequestMessage &reqMsg);
		bool			validateFieldValueCount(const std::string &name, const int count);
		bool			handleFieldValue(const std::string &name, const std::string &value, RequestMessage &reqMsg);
		std::string		cleanUpChunkedBody(const std::string &data, RequestMessage &reqMsg);
		std::string 	parseBody(const std::string &line, RequestMessage &reqMsg);
};

#endif
