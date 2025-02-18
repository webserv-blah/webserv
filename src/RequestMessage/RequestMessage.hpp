#pragma once

#include <string>
#include <map>
#include <vector>

typedef enum Method { INIT, GET, POST, DELETE } TypeMethod;
typedef enum connection { KEEP_ALIVE, CLOSE } TypeConnection;
typedef enum requestStatus {
	REQ_INIT,
	REQ_TOP_CRLF,		// CRLF(0,1)
	REQ_METHOD,			// 완료(1) => 완료이전에 판단중에 iss.peek() == EOF라면 기다려봐야함
	REQ_TARGET_URI,		// 완료(1)
	REQ_STARTLINE,		// start-line 완료
	REQ_HEADER_FIELD,	// 진행중(N)
	REQ_HEADER_CRLF,	// header-field 완료
	REQ_BODY,			// 진행중(N)
	REQ_DONE,
	REQ_ERROR
} TypeReqStatus;

class RequestMessage {
	public:
		typedef std::map<std::string, std::vector<std::string> >	TypeField;

		RequestMessage();
		RequestMessage(TypeMethod method, std::string target);
		~RequestMessage();

		bool			hasMethod() const;
		
		TypeMethod		getMethod() const;
		std::string		getTargetURI() const;
		TypeField		getFields() const;
		std::string		getBody() const;
		void			setMethod(TypeMethod method);
		void			setTargetURI(std::string targetURI);
		void			addFields(std::string field, std::vector<std::string> values);
		void			addBody(std::string bodyData);
		
		TypeReqStatus	getStatus() const;
		std:: string	getMetaHost() const;
		TypeConnection	getMetaConnection() const;
		ssize_t			getMetaContentLength() const;
		void			setStatus(TypeReqStatus status);
		void			setMetaHost(std::string value);
		void			setMetaConnection(std::string value);
		void			setMetaContentLength(std::string value);

		// 파싱 후, 결과 출력을 위한 함수
		void printResult() const;
		void printFields() const;
		void printBody() const;
		void printMetaData(void) const;
		
		// exception class 만드는게 유용할지도..

	private:
		TypeMethod		method_;
		std::string		targetURI_;
		TypeField		fieldLines_;
		std::string		body_;

		//meta data
		TypeReqStatus	status_;
		std::string		metaHost_;
		TypeConnection	metaConnection_;
		ssize_t			metaContentLength_;
		//...2
};
