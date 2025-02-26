#include "../utils.hpp"
#include "../../include/errorUtils.hpp"
#include <fstream>
#include <iostream>

// 에러 처리 방식 예시 코드
int main() {
    // 1. WARNING 레벨 에러 로그 예시 (파라미터 검증)
    int value = -10;
    if (value < 0) {
        webserv::logError(webserv::WARNING, "Invalid value", "Input is negative", 
                         "validateInput function");
        value = 0; // 기본값으로 복구
    }

	// 2. WARNING 레벨 에러 로그 예시
	ssize_t res = recv(curSession.getClientFd(), &buffer[0], BUFFER_SIZE, MSG_DONTWAIT);
	if (res == -1) {
		if (errno == EAGAIN
		||  errno == EWOULDBLOCK
		||  errno == EINTR) {
			webserv::logSystemError(webserv::WARNING, "recv failed", 
				"Client fd: " + std::to_string(curSession.getClientFd()), 
				"EventHandler::recvRequest");
			return READ_CONTINUE;
		}
	}
    
    // 3. ERROR 레벨 에러 로그 예시 (파일 열기 실패)
    std::ifstream file("non_existent_file.txt");
    if (!file.is_open()) {
        webserv::logError(webserv::ERROR, "File opening failed", "non_existent_file.txt", 
                         "errno " + std::to_string(errno) + ", " + std::strerror(errno));
        // 에러 발생 후 복구 시도 또는 대체 작업 수행
    }

    
    // 4. FATAL 에러 예외 발생 예시 (심각한 설정 오류)
    try {
        std::string configValue = "";
        if (configValue.empty()) {
            webserv::throwError("Missing required configuration", 
                              "server_name directive", "ConfigParser::parse");
        }
    } catch (const std::exception& e) {
        std::cerr << "Exception caught: " << e.what() << std::endl;
        return 1; // 프로그램 종료
    }
    
    // 5. 시스템 호출 예외 발생 예시 (메모리 할당 실패)
    try {
        void* ptr = malloc(1000000000000); // 매우 큰 크기로 의도적 실패
        if (!ptr) {
            webserv::throwSystemError("malloc", "Requested size: 1000000000000");
        }
        free(ptr);
    } catch (const std::exception& e) {
        std::cerr << "Exception caught: " << e.what() << std::endl;
        return 1; // 프로그램 종료
    }
    
    return 0;
}