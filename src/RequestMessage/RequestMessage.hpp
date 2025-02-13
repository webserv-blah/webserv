#pragma once

#include <string>
#include <map>
#include <vector>

enum Method {
	GET,
	POST,
	DELETE
};

class RequestMessage {
	public:
		typedef enum Method 									TypeMethod;
		typedef std::map<std::string, std::vector<std::string>>	TypeField;

		RequestMessage();
		RequestMessage(TypeMethod method, std::string target);
		~RequestMessage();

		void	setMethod(TypeMethod method);
		void	setTarget(std::string target);
		void	addFields(std::string field, std::vector<std::string> values);
		void	addBody(std::string bodyData);

	private:
		TypeMethod	method_;
		std::string	target_;
		TypeField	fieldLines_;
		std::string	body_;
		
		void	printFields(void) const;
};
