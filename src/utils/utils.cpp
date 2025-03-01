#include "utils.hpp"

extern volatile bool globalServerRunning;

// 시그널을 받으면 서버를 종료하도록 플래그를 설정하는 시그널 핸들러
void signalHandler(int signum) {
    std::cout << "Received signal " << signum << ", shutting down..." << std::endl;
    globalServerRunning = false;
}

// 시그널 핸들러 설정 함수
void setupSignalHandlers() {
	signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    signal(SIGQUIT, signalHandler);
}

namespace webserv {
    std::string errorMessage(EnumErrorLevel level, const std::string& cause, 
                            const std::string& context, const std::string& source) {
        std::ostringstream oss;
        
        // 에러 레벨에 따라 메시지 시작 설정
        switch (level) {
            case WARNING:
                oss << "[WARNING]";
                break;
            case ERROR:
                oss << "[ERROR]";
                break;
            case FATAL:
                oss << "[FATAL]";
                break;
        }
        
        // 표준 형식으로 메시지 조합
        oss << " : " << cause;
        
        if (!context.empty()) {
            oss << " - " << context;
        }
        
        if (!source.empty()) {
            oss << " (Source: " << source << ")";
        }
        
        return oss.str();
    }
}

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
	size_t sto_size_t(const std::string& str) {
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

	std::string strtrim(const std::string& str) {
		// 앞쪽 공백 제거
		size_t start = 0;
		while (start < str.size() && std::isspace(str[start]))
			++start;

		// 뒤쪽 공백 제거
		size_t end = str.size();
		while (end > start && std::isspace(str[end - 1]))
			--end;

		return str.substr(start, end - start);
	}

	std::string size_t_tos(size_t size) {
		std::ostringstream oss;
		oss << size;
		return oss.str();
	}

	std::string int_tos(int num) {
		std::ostringstream oss;
		oss << num;
		return oss.str();
	}
}