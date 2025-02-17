#pragma once

#include <string>
#include <map>
#include <vector>

#define CRLF "\r\n"
enum Method { INIT, GET, POST, DELETE };

class RequestMessage {
	public:
		typedef enum Method 									TypeMethod;
		typedef std::map<std::string, std::vector<std::string>>	TypeField;

		RequestMessage();
		RequestMessage(TypeMethod method, std::string target);
		~RequestMessage();

		bool		hasMethod() const;
		
		TypeMethod	getMethod() const;
		std::string	getTargetURI() const;
		TypeField	getFields() const;
		std::string	getBody() const;
		void		setMethod(TypeMethod method);
		void		setTargetURI(std::string targetURI);
		void		addFields(std::string field, std::vector<std::string> values);
		void		addBody(std::string bodyData);

	private:
		//should I record CRLF?
		TypeMethod	method_;
		std::string	targetURI_;
		TypeField	fieldLines_;
		std::string	body_;

		//message data
		ssize_t		contentLength;
		//...

		void	printFields(void) const;
};
