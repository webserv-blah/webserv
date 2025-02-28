#ifndef ERROR_UTILS_HPP
#define ERROR_UTILS_HPP

#include <string>
#include <stdexcept>
#include <sstream>
#include <iostream>
#include <cerrno>
#include <cstring>
#include "commonEnums.hpp"
#include "../utils/utils.hpp"

#ifdef DEBUG
	// 디버그 빌드용: 디버깅 메시지를 표준 에러(stderr)로 출력합니다.
    // 이 매크로는 디버그 정보를 로깅할 때 사용되며, 릴리스 빌드에서는 제거됩니다.
    #define DEBUG_LOG(x) do { std::clog << x << std::endl; } while(0)
#else
    // 일반(릴리스) 빌드용: 아무 동작도 하지 않음 (컴파일러에 의해 최적화되어 제거됨)
    #define DEBUG_LOG(x) ((void)0)
#endif

namespace webserv
{

// 에러 메시지 출력 (std::cerr 사용)
inline void logError(EnumErrorLevel level, const std::string& cause, 
                     const std::string& context = "", const std::string& source = "") {
    std::cerr << errorMessage(level, cause, context, source) << std::endl;
}

// 오류 상황에서 예외 던지기 (주로 FATAL 에러 레벨에 사용)
inline void throwError(const std::string& cause, 
                      const std::string& context = "", const std::string& source = "") {
    std::string errorMsg = errorMessage(FATAL, cause, context, source);
    throw std::runtime_error(errorMsg);
}

// 시스템 호출 실패에 대한 에러 메시지 생성 (errno 사용)
inline std::string systemErrorMessage(EnumErrorLevel level, const std::string& syscall, 
                                     const std::string& context = "", const std::string& location = "") {
    std::ostringstream oss;
    std::string cause = syscall + " failed";
    std::string source = (location.empty() ? "" : location + ", ") + 
                        "errno " + utils::size_t_tos(errno) + ", " + std::strerror(errno);
    return errorMessage(level, cause, context, source);
}

// 시스템 호출 실패 로깅 (errno 값과 설명 포함)
inline void logSystemError(EnumErrorLevel level, const std::string& syscall, 
                          const std::string& context = "", const std::string& location = "") {
    int err = errno; // 현재 errno 값 저장 (다른 함수 호출로 변경될 수 있음)
    std::ostringstream oss;
    std::string cause = syscall + " failed";
    std::string source = (location.empty() ? "" : location + ", ") + 
                        "errno " + utils::size_t_tos(err) + ", " + std::strerror(err);
    std::cerr << errorMessage(level, cause, context, source) << std::endl;
}

// 시스템 호출 실패로 예외 던지기 (errno 값과 설명 포함)
inline void throwSystemError(const std::string& syscall, const std::string& context = "", const std::string& location = "") {
    int err = errno; // 현재 errno 값 저장 (다른 함수 호출로 변경될 수 있음)
    std::ostringstream oss;
    std::string cause = syscall + " failed";
    std::string source = (location.empty() ? "" : location + ", ") + 
                        "errno " + utils::size_t_tos(err) + ", " + std::strerror(err);
    std::string errorMsg = errorMessage(FATAL, cause, context, source);
    throw std::runtime_error(errorMsg);
}

} // namespace webserv

#endif