#include "ConfigParser.hpp"

// LocationBlock 파서를 위한 함수들

// location 지시문의 경로를 파싱하는 함수
void ConfigParser::parseLocationPath(std::ifstream& configFile, std::string& locationPath) {
	std::string nextToken = getNextToken(configFile);
	if (nextToken.empty()) {
		throw std::runtime_error("Unexpected end of file in location directive");
	} else if (nextToken == ";") {
		throw std::runtime_error("Expected path for location directive");
	}
	// 읽어온 토큰을 경로로 설정
	locationPath = nextToken;
}

// methods 지시문을 파싱하는 함수
void ConfigParser::parseMethods(std::ifstream& configFile, std::vector<std::string>& methods) {
	std::string nextToken;
	while (true) {
		nextToken = peekNextToken(configFile);
		if (nextToken.empty()) {
			throw std::runtime_error("Unexpected end of file in methods directive");
		} else if (nextToken == ";") {
			if (methods.empty()) {
				// 메서드가 비어있으면 빈 문자열로 아무 메서드도 허용하지 않음을 나타냄
				methods.push_back("");
			}
			break;
		} else {
			if (!methods.empty() && methods.front() == "") {
				// 첫 번째 원소가 빈 문자열이면 제거
				methods.erase(methods.begin());
			}
			if (methods.end() == std::find(methods.begin(), methods.end(), nextToken)) {
				// 메서드가 중복되지 않으면 추가
				methods.push_back(nextToken);
			}
			nextToken = getNextToken(configFile);
		}
	}
}
