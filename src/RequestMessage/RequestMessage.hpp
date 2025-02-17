#pragma once

#include <string>
#include <map>
#include <vector>

#define CRLF "\r\n"
enum Method { INIT, GET, POST, DELETE };
////////////////buffersize를 굉장히 작게도 해봐야겠다
//해당 요소까지 완성되었음을 알림
enum requestStatus {
	REQ_INIT,				// 생성됨
	REQ_TOP_CRLF,			// CRLF 1개 허용됨
	REQ_METHOD,				// 완료 => 완료이전에 판단중에 iss.peek() == EOF라면 기다려봐야함
	REQ_TARGET_URI,			// 완료
	REQ_STARTLINE,			// start-line 완료
	REQ_STARTLINE_CRLF,		// start-line 이후 종료 상태 -> DONE으로 통합할지 고민
	REQ_HEADER_FIELD_ING,	// 
	REQ_HEADER_FIELD,		// 
	REQ_HEADER_CRLF,		// 
	REQ_BODY_ING,			// 
	REQ_BODY,				// 
	REQ_DONE,				// 
	REQ_ERROR				// 
};

class RequestMessage {
	public:
		typedef enum Method 										TypeMethod;
		typedef enum requestStatus									TypeReqStatus;
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
		ssize_t			getMetaConnection() const;
		ssize_t			getMetaContentLength() const;
		void			setStatus(TypeReqStatus status);
		void			setMetaHost(std::string value);
		void			setMetaConnection(ssize_t value);
		void			setMetaContentLength(ssize_t value);

		void printResult() const;//////////////////////////
		void printFields() const;//////////////////////////
		void printBody() const;//////////////////////////
	private:
		//0,1 -> 1개까지 허용
		TypeMethod		method_;
		std::string		targetURI_;
		//O -> 종료
		TypeField		fieldLines_;
		//O -> 종료 or body
		std::string		body_;

		//meta data
		TypeReqStatus	status_;
		std::string		metaHost_;
		ssize_t			metaConnection_;
		ssize_t			metaContentLength_;
		//...2
};
