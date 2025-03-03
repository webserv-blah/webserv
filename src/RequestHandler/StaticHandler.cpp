#include "StaticHandler.hpp"
#include "../include/commonEnums.hpp"
#include <dirent.h>
#include <errno.h>
#include <cctype>
#include <algorithm>
#include <map>
#include <sstream>
#include <fstream>   // 파일 쓰기/읽기
#include <cstdio>    // remove 함수

using namespace FileUtilities;

// ─────────────────────────────────────────────────────────────────────────────────────
// 유틸성 함수: 디렉토리 내 파일/폴더 목록 얻기
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
		// POSIX에서는 d_type이 항상 신뢰할 수는 없지만, 지원되는 경우 디렉토리면 '/' 추가
		if (ent->d_type == DT_DIR) {
			name.push_back('/');
		}
		entries.push_back(name);
	}
	closedir(dir);
	return entries;
}

// ─────────────────────────────────────────────────────────────────────────────────────
// 생성자/소멸자
StaticHandler::StaticHandler(const ResponseBuilder& responseBuilder)
: responseBuilder_(responseBuilder) {
}

StaticHandler::~StaticHandler() {
}

// ─────────────────────────────────────────────────────────────────────────────────────
// 메인 요청 처리 함수: GET / POST / DELETE 등 다양한 메소드를 분기
std::string StaticHandler::handleRequest(const RequestMessage& reqMsg, const RequestConfig& conf) {
	// 우선, 설정에서 현재 요청 메소드가 허용되는지 확인 (methods_ 벡터 사용)
	EnumMethod method = reqMsg.getMethod();
	if (!isMethodAllowed(method, conf)) {
		// 허용되지 않은 메소드
		return responseBuilder_.buildError(METHOD_NOT_ALLOWED, conf);
	}

	// GET, POST, DELETE 등 각각 분기 처리
	switch (method) {
		case GET:
			return handleGetRequest(reqMsg, conf);
		case POST:
			return handlePostRequest(reqMsg, conf);
		case DELETE:
			return handleDeleteRequest(reqMsg, conf);
		default:
			// 미구현 메소드인 경우 (PUT 등)
			return responseBuilder_.buildError(NOT_IMPLEMENTED, conf);
	}
}

// ─────────────────────────────────────────────────────────────────────────────────────
// GET 요청 처리
std::string StaticHandler::handleGetRequest(const RequestMessage& reqMsg, const RequestConfig& conf) {
	std::string uri = reqMsg.getTargetURI();
	std::string documentRoot = conf.root_;
	std::string fullPath = documentRoot + uri;

	// path 유효성 체크
	EnumValidationResult pathValidation = validatePath(fullPath);

	switch (pathValidation) {
		case VALID_PATH:
			// 디렉토리이면 인덱스 파일 혹은 autoIndex 처리
			return handleDirectory(fullPath, uri, conf);
		case VALID_FILE:
			// 정상 파일
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

// ─────────────────────────────────────────────────────────────────────────────────────
// POST 요청 처리 (단순화 버전)
//  - 요청 바디를 그대로 특정 경로에 쓰거나, 혹은 설정에 따라 처리
//  - 여기서는 단순히 uploadPath 위치에 파일을 생성/덮어쓰기 한다고 가정
std::string StaticHandler::handlePostRequest(const RequestMessage& reqMsg, const RequestConfig& conf) {
	std::string uploadPath = conf.uploadPath_;

	if (uploadPath.empty()) {
		return responseBuilder_.buildError(INTERNAL_SERVER_ERROR, conf);
	}
	// 유효한 파일 경로인지 간단히 확인 (디렉토리면 업로드 불가 등)
	EnumValidationResult pathValidation = validatePath(uploadPath);

	// 만약 디렉토리라면 바로 403
	if (pathValidation == PATH_NOT_FOUND) {
		return responseBuilder_.buildError(FORBIDDEN, conf);
	} 

	// 파일 이름 설정 (단순화된 방식, 실제로는 Content-Disposition 헤더 등을 참고할 수 있음)
	std::string filename = "uploaded_file.txt";
	if (uploadPath.size() > 0 && uploadPath[uploadPath.size() - 1] != '/') { //디렉토리 경로 끝에 / 없으면 추가.
		uploadPath.push_back('/');
	}
	std::string filePath = uploadPath + filename;

	// 파일 저장
	std::ofstream outFile(filePath.c_str(), std::ios::binary);
	if (!outFile) {
		return responseBuilder_.buildError(500, conf);
	}
	// 요청 바디 쓰기
	outFile << reqMsg.getBody();
	outFile.close();

	// 여기서는 단순히 OK 응답
	std::map<std::string, std::string> headers;
	headers["Content-Type"] = "text/plain";
	std::string body = "File uploaded: " + uploadPath;
	return responseBuilder_.build(OK, headers, body);
}

// ─────────────────────────────────────────────────────────────────────────────────────
// DELETE 요청 처리
//  - 파일만 삭제한다 가정 (디렉토리 삭제는 고려하지 않음)
//  - 실제로 remove() 호출
std::string StaticHandler::handleDeleteRequest(const RequestMessage& reqMsg, const RequestConfig& conf) {
	std::string uri = reqMsg.getTargetURI();
	std::string documentRoot = conf.root_;
	std::string fullPath = documentRoot + uri;

	// 경로 검사
	EnumValidationResult pathValidation = validatePath(fullPath);

	if (pathValidation == VALID_FILE) {
		// 파일 삭제 시도
		if (std::remove(fullPath.c_str()) == 0) {
			// 성공
			std::map<std::string, std::string> headers;
			std::string body = "File deleted: " + uri;
			return responseBuilder_.build(OK, headers, body);
		} else {
			// 삭제 실패
			return responseBuilder_.buildError(FORBIDDEN, conf);
		}
	}
	// 디렉토리거나 없으면 에러
	if (pathValidation == VALID_PATH) {
		// 디렉토리
		return responseBuilder_.buildError(FORBIDDEN, conf);
	}
	return responseBuilder_.buildError(NOT_FOUND, conf);
}

// ─────────────────────────────────────────────────────────────────────────────────────
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

	// index 파일이 존재하고 읽을 수 있다면
	if (indexValidation == VALID_FILE) {
		return handleFile(indexPath, conf);
	}
	// index 파일이 없고 autoIndex가 on이면 디렉토리 리스트 반환
	else if ((indexValidation == FILE_NOT_FOUND || indexValidation == PATH_NOT_FOUND) &&
		conf.autoIndex_.value() == 1)
	{
		return buildAutoIndexResponse(dirPath, uri);
	}

	// 나머지는 접근 불가
	return responseBuilder_.buildError(FORBIDDEN, conf);
}

// ─────────────────────────────────────────────────────────────────────────────────────
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

// ─────────────────────────────────────────────────────────────────────────────────────
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

// ─────────────────────────────────────────────────────────────────────────────────────
// 파일 확장자에 따른 MIME 타입 결정 (단순 매핑)
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

// ─────────────────────────────────────────────────────────────────────────────────────
// 현재 요청 메소드가 conf.methods_에 포함되는지 확인
bool StaticHandler::isMethodAllowed(EnumMethod method, const RequestConfig &conf) const {
	// RequestConfig::methods_는 문자열 벡터 예: {"GET", "POST", "DELETE"} 
	// EnumMethod를 문자열로 변환하거나, conf.methods_를 enum화하여 비교하는 식으로 구현 가능
	// 여기서는 간단히 문자열 비교 버전으로 가정
	std::string methodStr;
	switch (method) {
		case GET:       methodStr = "GET";    break;
		case POST:      methodStr = "POST";   break;
		case DELETE:   methodStr = "DELETE"; break;
		default:        methodStr = "UNKNOWN";break;
	}

	for (size_t i = 0; i < conf.methods_.size(); i++) {
		if (conf.methods_[i] == methodStr) {
			return true;
		}
	}
	return false;
}