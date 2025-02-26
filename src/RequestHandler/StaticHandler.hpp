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
#include <fstream>  // 파일 입출력에 필요 (post/delete)

class StaticHandler {
public:
    StaticHandler(const ResponseBuilder& responseBuilder);
    ~StaticHandler();

    // 요청 처리의 메인 진입점
    std::string handleRequest(const RequestMessage& reqMsg, const RequestConfig& conf);

private:
    const ResponseBuilder& responseBuilder_;

    // -- 메소드별 분기 처리 함수 --
    std::string handleGetRequest(const RequestMessage& reqMsg, const RequestConfig& conf);
    std::string handlePostRequest(const RequestMessage& reqMsg, const RequestConfig& conf);
    std::string handleDeleteRequest(const RequestMessage& reqMsg, const RequestConfig& conf);

    // -- 메소드 허용 여부 확인 --
    bool isMethodAllowed(EnumMethod method, const RequestConfig &conf) const;

    // -- 디렉토리 처리 --
    std::string handleDirectory(const std::string &dirPath,
                                const std::string &uri,
                                const RequestConfig &conf);

    // -- 파일 처리 --
    std::string handleFile(const std::string &filePath, const RequestConfig& conf);

    // -- redirection 처리 --
    std::string handleRedirction(const RequestConfig& conf);

    // -- autoindex 응답(디렉토리 목록) --
    std::string buildAutoIndexResponse(const std::string &dirPath, const std::string &uri);

    // -- MIME 타입 결정 --
    std::string determineContentType(const std::string &filePath) const;
};

#endif  // STATIC_HANDLER_HPP
