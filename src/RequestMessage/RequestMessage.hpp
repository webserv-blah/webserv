#ifndef REQUEST_MESSAGE_HPP
#define REQUEST_MESSAGE_HPP

#include <string>
#include <map>
#include <vector>

#define CR "\r"
#define LF "\n"
#define CRLF "\r\n"

enum EnumMethod { NONE, GET, HEAD, POST, DELETE };

typedef enum EnumConnection { KEEP_ALIVE, CLOSE } EnumConnect;
typedef enum EnumTransferEncoding { NONE_ENCODING, CHUNK } EnumTransEnc;
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
		~RequestMessage();

		EnumMethod		getMethod() const;
		std::string		getTargetURI() const;
		TypeField		getFields() const;
		std::string		getBody() const;
		size_t			getBodyLength() const;
		void			setMethod(const EnumMethod &method);
		void			setTargetURI(const std::string &targetURI);
		void			addFieldLine(const std::string &name, const std::vector<std::string> &values);
		void			addBody(const std::string &bodyData);
		
		EnumReqStatus	getStatus() const;
		std:: string	getMetaHost() const;
		EnumConnection	getMetaConnection() const;
		size_t			getMetaContentLength() const;
		EnumTransEnc	getMetaTransferEncoding() const;
		std:: string	getMetaContentType() const;
		void			setStatus(const EnumReqStatus &status);
		void			setMetaHost(const std::string &value);
		void			setMetaConnection(const EnumConnection &value);
		void			setMetaContentLength(const size_t &value);
		void			setMetaTransferEncoding(const EnumTransEnc &value);
		void			setMetaContentType(const std::string &value);

		void			resetHostField(const std::string &value);

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
		size_t			bodyLength_;

		// Header Field에 작성되어있던 메타데이터
		EnumReqStatus	status_;
		std::string		metaHost_;
		EnumConnect		metaConnection_;
		size_t			metaContentLength_;
		EnumTransEnc	metaTransferEncoding_;
		std::string		metaContentType_;
};

#endif
