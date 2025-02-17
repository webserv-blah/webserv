#include "ConfigParser.hpp"

// LocationBlock 파서를 위한 함수들

// location 지시문의 경로를 파싱하는 함수
void ConfigParser::parsePath(std::ifstream& configFile, std::string& path) {
	std::string nextToken = getNextToken(configFile);
	if (nextToken.empty()) {
		throw std::runtime_error("Unexpected end of file in location directive");
	} else if (nextToken == ";") {
		throw std::runtime_error("Expected path for location directive");
	}
	// 읽어온 토큰을 경로로 설정
	path = nextToken;
}

// methods 지시문을 파싱하는 함수
void ConfigParser::parseMethods(std::ifstream& configFile, std::vector<std::string>& methods) {
	std::string nextToken;
	while (true) {
		nextToken = peekNextToken(configFile);
		if (nextToken.empty()) {
			throw std::runtime_error("Unexpected end of file in methods directive");
		} else if (nextToken == ";") {
			break;
		} else {
			// 메서드 이름들을 methods 벡터에 추가
			methods.push_back(nextToken);
			nextToken = getNextToken(configFile);
		}
	}
}