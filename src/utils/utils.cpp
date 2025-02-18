#include "utils.hpp"

namespace utils {
	// 문자열을 int로 변환하는 함수
	int stoi(const std::string& str) {
		char* end; // 숫자열의 끝을 가리킬 포인터
		errno = 0; // 호출 전에 errno를 초기화
		long result = std::strtol(str.c_str(), &end, 10); // 10진수로 변환

		// 오버플로우 처리
		if (errno == ERANGE || result > INT_MAX || result < INT_MIN) {
			throw std::overflow_error("Integer overflow");
		}
		// 문자열이 유효한 정수 형식이 아닌 경우 예외를 던짐
		if (end == str.c_str() || *end != '\0') {
			throw std::invalid_argument("Invalid integer format");
		}

		// 변환 성공
		return static_cast<int>(result);
	}

	// 문자열을 size_t로 변환하는 함수
	size_t stosizet(const std::string& str) {
		char* end;
		errno = 0;
		unsigned long result = std::strtoul(str.c_str(), &end, 10);

		// 오버플로우 처리
		if (errno == ERANGE || result > std::numeric_limits<size_t>::max()) {
			throw std::overflow_error("size_t overflow");
		}
		// 유효하지 않은 포맷 처리
		if (end == str.c_str() || *end != '\0') {
			throw std::invalid_argument("Invalid size_t format");
		}

		return static_cast<size_t>(result);
	}
}