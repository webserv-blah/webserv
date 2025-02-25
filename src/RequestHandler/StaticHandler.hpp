#ifndef STATIC_HANDLER_HPP
#define STATIC_HANDLER_HPP

#include "GlobalConfig.hpp"
#include "ClientSession.hpp"
#include "RequestMessage.hpp"
#include "ResponseBuilder.hpp"
#include "file_utils.hpp"
#include <sstream>
#include <vector>
#include <string>
#include <map>

class StaticHandler {
public:
    StaticHandler(const ResponseBuilder& responseBuilder);
    ~StaticHandler();

    // 요청 처리의 메인 진입점
    std::string handleRequest(const RequestMessage& reqMsg, const RequestConfig& conf);

private:
    const ResponseBuilder& responseBuilder_;

    // 디렉토리 처리
    std::string handleDirectory(const std::string &dirPath,
                                const std::string &uri,
                                const RequestConfig &conf);

    // 파일 처리
    std::string handleFile(const std::string &filePath, const RequestConfig& conf);

    // redirection 처리
    std::string handleRedirction(const RequestConfig& conf);

    // index.html 등 특정 파일 응답
    std::string serveFile(const std::string &filePath, const std::string &contentType, const RequestConfig& conf);

    // autoindex 응답(디렉토리 목록)
    std::string buildAutoIndexResponse(const std::string &dirPath, const std::string &uri);

    // MIME 타입 결정
    std::string determineContentType(const std::string &filePath) const;
};

#endif
