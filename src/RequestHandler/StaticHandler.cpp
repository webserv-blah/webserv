#include "StaticHandler.hpp"
#include "commonEnums.hpp"
#include <dirent.h>
#include <errno.h>
#include <cctype>
#include <algorithm>
#include <map>
#include <sstream>

using namespace FileUtilities; // validatePath, readFile 등이 이 네임스페이스에 있다고 가정

// POSIX의 dirent.h를 이용하여 디렉토리 목록을 반환
static std::vector<std::string> getDirectoryListing(const std::string &directoryPath) {
	std::vector<std::string> entries;
	DIR* dir = opendir(directoryPath.c_str());
	if (!dir) {
		// 디렉토리 오픈 실패 시 빈 목록 반환
		return entries;
	}
	struct dirent* ent;
	while ((ent = readdir(dir)) != NULL) {
		std::string name(ent->d_name);
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
	if (conf.returnStatus_ >= 100) {
		if (conf.returnStatus_ == OK) {
			//location block의 리턴 지시자가 text로 오는 경우. ex) "return OK text" ??
		} else if (!conf.returnUrl_.empty() && (conf.returnStatus_ == FOUND || conf.returnStatus_ == MOVED_PERMANENTLY)) {
			return handleRedirction(conf);
		} else {
			return responseBuilder_.buildError(conf.returnStatus_, conf);
		}
	}
	
	std::string uri = reqMsg.getTargetURI();
	std::string documentRoot = conf.root_;
	std::string fullPath = documentRoot + uri;
	
	// path 유효성 체크
	EnumValidationResult pathValidation = validatePath(fullPath);
	
	switch (pathValidation) {
		case VALID_PATH:
		return handleDirectory(fullPath, uri, conf);
		case VALID_FILE:
		return handleFile(fullPath, conf);
		case PATH_NOT_FOUND:
		case FILE_NOT_FOUND:
		return responseBuilder_.buildError(NOT_FOUND, conf);
		case PATH_NO_PERMISSION:
		case FILE_NO_READ_PERMISSION:
		case FILE_NO_EXEC_PERMISSION:
		return responseBuilder_.buildError(FORBIDDEN, conf);
		default:
		return responseBuilder_.buildError(INTERNAL_SERVER_ERROR, conf);
	}
}

// 디렉토리 처리: index.html 우선, autoIndex 기능, 그 외 403 반환
std::string StaticHandler::handleDirectory(const std::string &dirPath,
	const std::string &uri,
	const RequestConfig &conf)
	{
		std::string indexPath = dirPath;
		if (!indexPath.empty() && indexPath[indexPath.size()-1] != '/') {
			indexPath.push_back('/');
		}
		indexPath += conf.indexFile_;
		
		EnumValidationResult indexValidation = validatePath(indexPath);
		
		if (indexValidation == VALID_FILE) {
			return handleFile(indexPath, conf);
		} else if ((indexValidation == FILE_NOT_FOUND || indexValidation == PATH_NOT_FOUND) &&
		conf.autoIndex_.value() == 1) {
			return buildAutoIndexResponse(dirPath, uri);
		}
		
		return responseBuilder_.buildError(FORBIDDEN, conf);
	}
	
	// redirection 처리
	std::string StaticHandler::handleRedirction(const RequestConfig& conf) {
		std::string location = conf.returnUrl_;
		std::map<std::string, std::string> headers;
		headers["Location:"] = location;
		return responseBuilder_.build(conf.returnStatus_, headers, "");
	}
	
// 파일 처리: 파일 읽기, MIME 타입 결정 후 응답
std::string StaticHandler::handleFile(const std::string &filePath, const RequestConfig& conf) {
	std::string content = readFile(filePath);
	if (content.empty()) {
		return responseBuilder_.buildError(NOT_FOUND, conf);
	}
	std::map<std::string, std::string> headers;
	headers["Content-Type"] = determineContentType(filePath);
	return responseBuilder_.build(OK, headers, content);
}

// autoIndex가 켜진 경우, 디렉토리 목록을 HTML로 구성하여 반환
std::string StaticHandler::buildAutoIndexResponse(const std::string &dirPath, const std::string &uri)
{
	std::vector<std::string> entries = getDirectoryListing(dirPath);
	std::ostringstream body;
	body << "<html><head><meta charset=\"UTF-8\"/></head><body>";
	body << "<h1>Index of " << uri << "</h1><ul>";

	bool endsWithSlash = (!uri.empty() && uri[uri.size()-1] == '/');
	for (std::vector<std::string>::const_iterator it = entries.begin(); it != entries.end(); ++it) {
		body << "<li><a href=\"" << uri 
			 << (endsWithSlash ? "" : "/")
			 << *it << "\">" << *it << "</a></li>";
	}
	body << "</ul></body></html>";
	std::map<std::string, std::string> headers;
	headers["Content-Type"] = "text/html";
	return responseBuilder_.build(OK, headers, body.str());
}

// 파일 확장자에 따른 MIME 타입 결정
std::string StaticHandler::determineContentType(const std::string &filePath) const {
	std::string::size_type pos = filePath.find_last_of('.');
	if (pos == std::string::npos) {
		return "application/octet-stream";
	}
	std::string ext = filePath.substr(pos + 1);
	// 확장자를 소문자로 변환
	for (std::string::iterator it = ext.begin(); it != ext.end(); ++it) {
		*it = static_cast<char>(tolower(*it));
	}

	// 확장자와 MIME 타입 매핑
	std::map<std::string, std::string> mimeMap;
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

	std::map<std::string, std::string>::const_iterator iter = mimeMap.find(ext);
	if (iter != mimeMap.end()) {
		return iter->second;
	}
	return "application/octet-stream";
}
