#include "ConfigParser.hpp"

// ReqHandleConf 파서를 위한 함수들

// error_page 지시문을 파싱하는 함수
void ConfigParser::parseErrorPage(std::ifstream& configFile, std::map<int, std::string> &errorPages) {
	std::vector<int> errorCodes;
	std::string      nextToken;

	while (true) {
		nextToken = getNextToken(configFile);
		if (nextToken.empty()) {
			throw std::runtime_error("Unexpected end of file in error_page directive");
		} else if (utils::all_of(nextToken.begin(), nextToken.end(), ::isdigit)) {
			// 에러 코드는 3자리여야 함
			if (nextToken.size() == 3) {
				int errorCode = std::stoi(nextToken);
				errorCodes.push_back(errorCode);
			} else {
				throw std::runtime_error("Invalid error code for error_page directive");
			}
		} else {
			break;
		}
	}

	if (nextToken == ";") {
		throw std::runtime_error("Expected error page URL for error_page directive");
	} else if (errorCodes.empty()) {
		throw std::runtime_error("Expected at least one error code for error_page directive");
	}

	// 다음 토큰은 error page URL임
	std::string errorPageUrl = nextToken;
	for (size_t i = 0; i < errorCodes.size(); ++i) {
		errorPages[errorCodes[i]] = errorPageUrl;
	}
}

// return 지시문을 파싱하는 함수
void ConfigParser::parseReturn(std::ifstream& configFile, std::string& returnUrl, int& returnStatus) {
	// 상태 코드 토큰을 읽음
	std::string nextToken = getNextToken(configFile);
	if (nextToken.empty()) {
		throw std::runtime_error("Unexpected end of file in return directive");
	}
	
	// 토큰이 숫자로만 구성되어 있는지 확인하고, 3자리여야 함
	if (utils::all_of(nextToken.begin(), nextToken.end(), ::isdigit)) {
		if (nextToken.size() == 3) {
			// 토큰을 int로 변환 후 returnStatus에 저장
			returnStatus = std::stoi(nextToken);
		} else {
			// 3자리가 아니면 예외 발생
			throw std::runtime_error("Invalid HTTP status code for return directive");
		}
	} else {
		// 숫자로만 구성되어 있지 않으면 예외 발생
		throw std::runtime_error("Expected HTTP status code for return directive");
	}

	// URL 토큰을 읽음
	nextToken = getNextToken(configFile);
	if (nextToken.empty()) {
		throw std::runtime_error("Unexpected end of file in return directive");
	} else if (nextToken == ";") {
		throw std::runtime_error("Expected URL for return directive");
	}
	// 읽어온 토큰을 return URL로 설정
	returnUrl = nextToken;
}

// root 지시문을 파싱하는 함수
void ConfigParser::parseRoot(std::ifstream& configFile, std::string& root) {
	std::string nextToken = getNextToken(configFile);
	if (nextToken.empty()) {
		throw std::runtime_error("Unexpected end of file in root directive");
	} else if (nextToken == ";") {
		throw std::runtime_error("Expected root path for root directive");
	}
	// 읽어온 토큰을 root 경로로 설정
	root = nextToken;
}

// index 지시문을 파싱하는 함수
void ConfigParser::parseIndexFile(std::ifstream& configFile, std::string& indexFile) {
	std::string nextToken = getNextToken(configFile);
	if (nextToken.empty()) {
		throw std::runtime_error("Unexpected end of file in index directive");
	} else if (nextToken == ";") {
		throw std::runtime_error("Expected index file name for index directive");
	}
	// 읽어온 토큰을 index file 이름으로 설정
	indexFile = nextToken;
}

// upload_path 지시문을 파싱하는 함수
void ConfigParser::parseUploadPath(std::ifstream& configFile, std::string& uploadPath) {
	std::string nextToken = getNextToken(configFile);
	if (nextToken.empty()) {
		throw std::runtime_error("Unexpected end of file in upload_path directive");
	} else if (nextToken == ";") {
		throw std::runtime_error("Expected upload path for upload_path directive");
	}
	// 읽어온 토큰을 upload 경로로 설정
	uploadPath = nextToken;
}

// cgi_extension 지시문을 파싱하는 함수
void ConfigParser::parseCgiExtension(std::ifstream& configFile, std::string& cgiExtension) {
	std::string nextToken = getNextToken(configFile);
	if (nextToken.empty()) {
		throw std::runtime_error("Unexpected end of file in cgi_extension directive");
	} else if (nextToken == ";") {
		throw std::runtime_error("Expected CGI extension for cgi_extension directive");
	} else if (nextToken != ".php") {
		// CGI 확장자가 .php가 아니면 예외 발생
		throw std::runtime_error("Unsupported CGI extension");
	}
	// 읽어온 토큰을 CGI 확장자로 설정
	cgiExtension = nextToken;
}

// client_max_body_size 지시문을 파싱하는 함수
void ConfigParser::parseClientMaxBodySize(std::ifstream& configFile, Optional<size_t>& clientMaxBody) {
	std::string nextToken = getNextToken(configFile);
	if (nextToken.empty()) {
		throw std::runtime_error("Unexpected end of file in client_max_body_size directive");
	} else if (utils::all_of(nextToken.begin(), nextToken.end(), ::isdigit)) {
		// 토큰을 unsigned long 형태로 변환 후 clientMaxBody에 저장
		clientMaxBody = std::stoul(nextToken);
	} else {
		throw std::runtime_error("Expected numeric value for client_max_body_size directive");
	}
}

// autoindex 지시문을 파싱하는 함수
void ConfigParser::parseAutoIndex(std::ifstream& configFile, Optional<bool>& autoIndex) {
	std::string nextToken = getNextToken(configFile);
	if (nextToken.empty()) {
		throw std::runtime_error("Unexpected end of file in autoindex directive");
	} else if (nextToken == "on") {
		// 'on'이면 autoIndex를 true로 설정
		autoIndex.setValue(true);
	} else if (nextToken == "off") {
		// 'off'이면 autoIndex를 false로 설정
		autoIndex.setValue(false);
	} else {
		throw std::runtime_error("Expected 'on' or 'off' for autoindex directive");
	}
}
