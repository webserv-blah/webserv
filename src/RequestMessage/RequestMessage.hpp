#pragma once

#include <string>
#include <map>
#include <vector>

typedef enum Method { NONE, GET, POST, DELETE } TypeMethod;
typedef enum connection { KEEP_ALIVE, CLOSE } TypeConnection;
typedef enum requestStatus {
	REQ_INIT,
	REQ_TOP_CRLF,		// CRLF(0,1)
	REQ_METHOD,			// 완료(1)
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

		TypeMethod		getMethod() const;
		std::string		getTargetURI() const;
		TypeField		getFields() const;
		std::string		getBody() const;
		void			setMethod(const TypeMethod &method);
		void			setTargetURI(const std::string &targetURI);
		void			addFields(std::string field, std::vector<std::string> values);
		void			addBody(std::string bodyData);
		
		TypeReqStatus	getStatus() const;
		std:: string	getMetaHost() const;
		TypeConnection	getMetaConnection() const;
		ssize_t			getMetaContentLength() const;
		void			setStatus(const TypeReqStatus &status);
		void			setMetaHost(const std::string &value);
		void			setMetaConnection(const std::string &value);
		void			setMetaContentLength(const std::string &value);

		// 파싱 후, 결과 출력을 위한 함수
		void printResult() const;
		void printFields() const;
		void printBody() const;
		void printMetaData(void) const;
		
		// MUST TO DO: exception class 만드는게 유용할지도..

	private:
		TypeMethod		method_;
		std::string		targetURI_;
		TypeField		fieldLines_;
		std::string		body_;

		// Header Field에 작성되어있던 메타데이터
		TypeReqStatus	status_;
		std::string		metaHost_;
		TypeConnection	metaConnection_;
		ssize_t			metaContentLength_;
};
