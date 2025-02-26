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

namespace webserv
{
// 에러 레벨 상수를 네임스페이스 내에서도 사용할 수 있도록 함
using ::WARNING;  // 경고 (계속 진행 가능)
using ::ERROR;    // 오류 (복구 필요)
using ::FATAL;    // 치명적 오류 (프로그램 종료)

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
                        "errno " + std::to_string(errno) + ", " + std::strerror(errno);
    return errorMessage(level, cause, context, source);
}

// 시스템 호출 실패 로깅 (errno 값과 설명 포함)
inline void logSystemError(EnumErrorLevel level, const std::string& syscall, 
                          const std::string& context = "", const std::string& location = "") {
    int err = errno; // 현재 errno 값 저장 (다른 함수 호출로 변경될 수 있음)
    std::ostringstream oss;
    std::string cause = syscall + " failed";
    std::string source = (location.empty() ? "" : location + ", ") + 
                        "errno " + std::to_string(err) + ", " + std::strerror(err);
    std::cerr << errorMessage(level, cause, context, source) << std::endl;
}

// 시스템 호출 실패로 예외 던지기 (errno 값과 설명 포함)
inline void throwSystemError(const std::string& syscall, const std::string& context = "", const std::string& location = "") {
    int err = errno; // 현재 errno 값 저장 (다른 함수 호출로 변경될 수 있음)
    std::ostringstream oss;
    std::string cause = syscall + " failed";
    std::string source = (location.empty() ? "" : location + ", ") + 
                        "errno " + std::to_string(err) + ", " + std::strerror(err);
    std::string errorMsg = errorMessage(FATAL, cause, context, source);
    throw std::runtime_error(errorMsg);
}

} // namespace webserv

#endif