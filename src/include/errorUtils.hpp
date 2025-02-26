#ifndef ERROR_UTILS_HPP
#define ERROR_UTILS_HPP

#include <string>
#include <stdexcept>
#include <sstream>
#include <iostream>
#include <cerrno>
#include <cstring>
#include "commonEnums.hpp"

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
                                     const std::string& context = "") {
    std::ostringstream oss;
    std::string cause = syscall + " failed";
    std::string source = "errno " + std::to_string(errno) + ", " + std::strerror(errno);
    return errorMessage(level, cause, context, source);
}

// 시스템 호출 실패 로깅
inline void logSystemError(EnumErrorLevel level, const std::string& syscall, 
                          const std::string& context = "") {
    std::cerr << systemErrorMessage(level, syscall, context) << std::endl;
}

// 시스템 호출 실패로 예외 던지기
inline void throwSystemError(const std::string& syscall, const std::string& context = "") {
    std::string errorMsg = systemErrorMessage(FATAL, syscall, context);
    throw std::runtime_error(errorMsg);
}

} // namespace webserv

#endif