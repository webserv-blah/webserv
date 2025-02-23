#ifndef CGIHANDLER_HPP
#define CGIHANDLER_HPP

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <cerrno>
#include <cstdlib>
#include <cstdio>
#include <fcntl.h>
#include <sstream>
#include <algorithm>
#include <string>
#include <vector>

class CgiHandler {
public:
    // ResponseBuilder를 받음 (에러 시 buildError 사용)
    CgiHandler(const ResponseBuilder& responseBuilder);
    ~CgiHandler();

    // CGI를 실행하고, 자식 프로세스의 표준 출력 내용을 그대로 문자열로 반환.
    // 만약 에러 발생 시, responseBuilder.buildError(...) 를 통해 에러응답 문자열을 반환.
    std::string handleRequest(const RequestMessage& reqMsg, const RequestConfig& conf);

private:
    const ResponseBuilder& responseBuilder_; // 에러 응답 생성에 사용

    // CGI 환경 변수 구성
    std::vector<std::string> buildCgiEnv(const RequestMessage& reqMsg,
                                         const std::string& scriptPath,
                                         const RequestConfig& conf);

    // fork/execve로 CGI 스크립트 실행, 자식 STDOUT 전부 읽어서 반환
    // - 실패 시 빈 문자열
    std::string executeCgi(const std::string& scriptPath,
                           const std::vector<std::string>& cgiEnv,
                           const std::string& bodyForCgi);
};

#endif