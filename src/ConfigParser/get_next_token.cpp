#include "ConfigParser.hpp"

// 파일에서 다음 토큰을 읽어오는 함수
std::string ConfigParser::getNextToken(std::ifstream& configFile) {
	// 반환할 토큰
	std::string token;

	if (!configFile.good()) {
		// 파일 스트림이 열려있지 않거나, EOF에 도달한 경우
		return token; // 빈 문자열 반환
	}

	// 1) 선행 공백 건너뛰기
	while (true) {
		int c = configFile.peek(); // 다음 문자를 추출하지 않고 확인
		if (c == EOF) {
			// 더 이상 읽을 데이터가 없음
			return "";
		}

		if (!std::isspace(static_cast<unsigned char>(c))) {
			// 다음 문자가 공백이 아닌 경우; 루프 탈출
			break;
		}
		// 공백을 소비/무시
		configFile.get();
	}

	// 2) 다음 문자가 특수 단일 문자 토큰인지 확인
	{
		char c = static_cast<char>(configFile.peek());
		if (c == '{' || c == '}' || c == ';') {
			// 특수 토큰
			token.push_back(c);
			// 소비
			configFile.get();
			return token;
		}
	}

	// 3) 그렇지 않으면 공백이나 특수 문자를 만날 때까지 문자 읽기
	while (true) {
		int c = configFile.peek();
		if (c == EOF) {
			// 더 이상 문자가 없음
			break;
		}
		char ch = static_cast<char>(c);

		// 공백이나 특수 문자를 만나면 토큰 읽기 중지
		if (std::isspace(static_cast<unsigned char>(ch)) ||
			ch == '{' || ch == '}' || ch == ';') {
			break;
		}

		// 그렇지 않으면 이 문자를 토큰에 추가
		token.push_back(ch);
		// 스트림에서 소비
		configFile.get();
	}

	return token;
}

// 파일에서 다음 토큰을 미리 확인하는 함수
std::string ConfigParser::peekNextToken(std::ifstream& configFile) {
	// 현재 스트림 위치 저장
	std::streampos originalPos = configFile.tellg();
	// 현재 스트림 상태 저장
	std::ios::iostate originalState = configFile.rdstate();

	// 다음 토큰 읽기
	std::string token = getNextToken(configFile);

	// 위치와 상태 복원
	configFile.clear();                 // 가능한 EOF 또는 실패 플래그 지우기
	configFile.seekg(originalPos);      // 저장된 위치로 돌아가기
	configFile.setstate(originalState); // 원래 상태 복원

	return token;
}
