#pragma once

#include <string>
#include <map>
#include <vector>

enum EnumMethod { NONE, GET, POST, DELETE };
enum EnumConnection { KEEP_ALIVE, CLOSE };
typedef enum EnumRequestStatus {
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
} EnumReqStatus;

class RequestMessage {
	public:
		typedef std::map<std::string, std::vector<std::string> >	TypeField;

		RequestMessage();
		RequestMessage(EnumMethod method, std::string target);
		~RequestMessage();

		EnumMethod		getMethod() const;
		std::string		getTargetURI() const;
		TypeField		getFields() const;
		std::string		getBody() const;
		void			setMethod(const EnumMethod &method);
		void			setTargetURI(const std::string &targetURI);
		void			addFields(std::string field, std::vector<std::string> values);
		void			addBody(std::string bodyData);
		
		EnumReqStatus	getStatus() const;
		std:: string	getMetaHost() const;
		EnumConnection	getMetaConnection() const;
		ssize_t			getMetaContentLength() const;
		void			setStatus(const EnumReqStatus &status);
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
		EnumMethod		method_;
		std::string		targetURI_;
		TypeField		fieldLines_;
		std::string		body_;

		// Header Field에 작성되어있던 메타데이터
		EnumReqStatus	status_;
		std::string		metaHost_;
		EnumConnection	metaConnection_;
		ssize_t			metaContentLength_;
};
