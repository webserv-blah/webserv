#include "StaticHandler.hpp"
#include <dirent.h>
#include <errno.h>
#include <cctype>
#include <algorithm>
#include <map>
#include <sstream>

// using namespace는 C++98에서도 사용 가능
using namespace std;
using namespace FileUtilities; // validatePath, readFile 등이 이 네임스페이스에 있다고 가정

// POSIX의 dirent.h를 이용하여 디렉토리 목록을 반환
static vector<string> getDirectoryListing(const string &directoryPath) {
    vector<string> entries;
    DIR* dir = opendir(directoryPath.c_str());
    if (!dir) {
        // 디렉토리 오픈 실패 시 빈 목록 반환
        return entries;
    }
    struct dirent* ent;
    while ((ent = readdir(dir)) != NULL) {
        string name(ent->d_name);
        // 현재 디렉토리(.)와 상위 디렉토리(..)는 제외
        if (name == "." || name == "..") {
            continue;
        }
        // POSIX에서는 d_type가 항상 지원되지는 않지만, 지원되는 경우 디렉토리면 '/' 추가
        if (ent->d_type == DT_DIR) {
            name.push_back('/');
        }
        entries.push_back(name);
    }
    closedir(dir);
    return entries;
}

// 생성자와 소멸자
StaticHandler::StaticHandler(const ResponseBuilder& responseBuilder)
    : responseBuilder_(responseBuilder) {
}

StaticHandler::~StaticHandler() {
}

// 요청 처리의 메인 진입점
std::string StaticHandler::handleRequest(const RequestMessage& reqMsg, const RequestConfig& conf) {
    string uri = reqMsg.getTargetURI();      // ex) "/images/logo.png"
    string documentRoot = conf.root_;          // ex) "/var/www/html"
    string fullPath = documentRoot + uri;      // ex) "/var/www/html/images/logo.png"

    // path 유효성 체크 (validatePath는 file_utils.hpp에 구현되어 있다고 가정)
    EnumValidationResult pathValidation = validatePath(fullPath);

    switch (pathValidation) {
        case VALID_PATH:
            return handleDirectory(fullPath, uri, conf);
        case VALID_FILE:
            return handleFile(fullPath, conf);
        case PATH_NOT_FOUND:
        case FILE_NOT_FOUND:
            return responseBuilder_.buildError(404, conf);
        case PATH_NO_PERMISSION:
        case FILE_NO_READ_PERMISSION:
        case FILE_NO_EXEC_PERMISSION:
            return responseBuilder_.buildError(403, conf);
        default:
            return responseBuilder_.buildError(500, conf);
    }
}

// 디렉토리 처리: index.html 우선, autoIndex 기능, 그 외 403 반환
std::string StaticHandler::handleDirectory(const std::string &dirPath,
                                             const std::string &uri,
                                             const RequestConfig &conf)
{
    string indexPath = dirPath;
    if (!indexPath.empty() && indexPath[indexPath.size()-1] != '/') {
        indexPath.push_back('/');
    }
    indexPath += "index.html";

    EnumValidationResult indexValidation = validatePath(indexPath);

    if (indexValidation == VALID_FILE) {
        return serveFile(indexPath, "text/html", conf);
    } else if ((indexValidation == FILE_NOT_FOUND || indexValidation == PATH_NOT_FOUND) &&
               conf.autoIndex_.value() == 1) {
        return buildAutoIndexResponse(dirPath, uri);
    }

    return responseBuilder_.buildError(403, conf);
}

// 파일 처리: 파일 읽기, MIME 타입 결정 후 응답
std::string StaticHandler::handleFile(const std::string &filePath, const RequestConfig& conf) {
    string content = readFile(filePath);
    if (content.empty()) {
        return responseBuilder_.buildError(404, conf);
    }
    string contentType = determineContentType(filePath);
    return responseBuilder_.build(200, contentType, content);
}

// 특정 파일(예: index.html) 서빙용 헬퍼 함수
std::string StaticHandler::serveFile(const std::string &filePath, const std::string &contentType, const RequestConfig& conf) {
    string content = readFile(filePath);
    if (content.empty()) {
        return responseBuilder_.buildError(404, conf);
    }
    return responseBuilder_.build(200, contentType, content);
}

// autoIndex가 켜진 경우, 디렉토리 목록을 HTML로 구성하여 반환
std::string StaticHandler::buildAutoIndexResponse(const std::string &dirPath, const std::string &uri)
{
    vector<string> entries = getDirectoryListing(dirPath);
    ostringstream body;
    body << "<html><head><meta charset=\"UTF-8\"/></head><body>";
    body << "<h1>Index of " << uri << "</h1><ul>";

    bool endsWithSlash = (!uri.empty() && uri[uri.size()-1] == '/');
    for (vector<string>::const_iterator it = entries.begin(); it != entries.end(); ++it) {
        body << "<li><a href=\"" << uri 
             << (endsWithSlash ? "" : "/")
             << *it << "\">" << *it << "</a></li>";
    }
    body << "</ul></body></html>";

    return responseBuilder_.build(200, "text/html; charset=UTF-8", body.str());
}

// 파일 확장자에 따른 MIME 타입 결정
std::string StaticHandler::determineContentType(const std::string &filePath) const {
    string::size_type pos = filePath.find_last_of('.');
    if (pos == string::npos) {
        return "application/octet-stream";
    }
    string ext = filePath.substr(pos + 1);
    // 확장자를 소문자로 변환
    for (string::iterator it = ext.begin(); it != ext.end(); ++it) {
        *it = static_cast<char>(tolower(*it));
    }

    // 확장자와 MIME 타입 매핑
    map<string, string> mimeMap;
    mimeMap["html"] = "text/html";
    mimeMap["htm"]  = "text/html";
    mimeMap["css"]  = "text/css";
    mimeMap["js"]   = "application/javascript";
    mimeMap["png"]  = "image/png";
    mimeMap["jpg"]  = "image/jpeg";
    mimeMap["jpeg"] = "image/jpeg";
    mimeMap["gif"]  = "image/gif";
    mimeMap["txt"]  = "text/plain";
    mimeMap["json"] = "application/json";
    mimeMap["svg"]  = "image/svg+xml";
    mimeMap["ico"]  = "image/x-icon";

    map<string, string>::const_iterator iter = mimeMap.find(ext);
    if (iter != mimeMap.end()) {
        return iter->second;
    }
    return "application/octet-stream";
}
