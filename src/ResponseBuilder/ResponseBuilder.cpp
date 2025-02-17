#include "ResponseBuilder.hpp"
#include <ctime>
#include <fstream>
#include <sstream>

ResponseBuilder::ResponseBuilder(const ReqHandleConf& conf)
	: currConf_(conf), statusCode_(200), reasonPhrase_("OK")
{
}

ResponseBuilder::~ResponseBuilder() {
}

void ResponseBuilder::setStatus(int code, const std::string &reason) {
	statusCode_ = code;
	reasonPhrase_ = reason;
}

void ResponseBuilder::addHeader(const std::string &key, const std::string &value) {
	headers_[key] = value;
}

void ResponseBuilder::setBody(const std::string &bodyContent) {
	body_ = bodyContent;
	// body 내용 길이에 맞춰 Content-Length 헤더 설정
	std::ostringstream oss;
	oss << body_.size();
	headers_["Content-Length"] = oss.str();
}

// 파일 내용 읽기 (주로 buildError()에서 사용)
bool ResponseBuilder::readFileContentToString(const std::string &filePath, std::string &outContent) const {
	std::ifstream file(filePath.c_str(), std::ios::in | std::ios::binary);
	if (!file.is_open()) {
		return false;
	}
	std::ostringstream oss;
	oss << file.rdbuf();
	outContent = oss.str();
	return true;
}

// HTTP 상태코드 -> 기본 Reason-Phrase
std::string ResponseBuilder::getReasonPhrase(int code) const {
	switch (code) {
		case 200: return "OK";
		case 400: return "Bad Request";
		case 403: return "Forbidden";
		case 404: return "Not Found";
		case 405: return "Method Not Allowed";
		case 500: return "Internal Server Error";
		default:  return "Error";
	}
}

// RFC1123 형식 날짜
std::string ResponseBuilder::getCurrentDate() const {
	std::time_t now = std::time(NULL);
	char buf[100];
	std::strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S GMT", std::gmtime(&now));
	return std::string(buf);
}

// 최종 HTTP 응답 문자열 생성
std::string ResponseBuilder::assembleResponse(int statusCode,
											  const std::string &reasonPhrase,
											  const std::map<std::string, std::string> &headers,
											  const std::string &body) const
{
	std::ostringstream oss;
	oss << "HTTP/1.1 " << statusCode << " " << reasonPhrase << "\r\n";
	oss << "Date: " << getCurrentDate() << "\r\n";

	// 헤더 출력
	std::map<std::string, std::string>::const_iterator it = headers.begin();
	for (; it != headers.end(); ++it) {
		oss << it->first << ": " << it->second << "\r\n";
	}

	// 헤더와 바디 사이 빈 줄
	oss << "\r\n";
	oss << body;
	return oss.str();
}

// 일반 응답
std::string ResponseBuilder::build() const {
	// 혹시 Body가 비었는데 Content-Length가 설정 안됐으면 0으로 세팅
	if (body_.empty()) {
		if (headers_.find("Content-Length") == headers_.end()) {
			std::ostringstream oss;
			oss << 0;
			// build()가 const 함수이므로 const_cast
			const_cast<ResponseBuilder*>(this)->headers_["Content-Length"] = oss.str();
		}
	}
	return assembleResponse(statusCode_, reasonPhrase_, headers_, body_);
}

// 에러 응답 (커스텀 에러 페이지 -> 기본 에러 페이지 -> fallback)
std::string ResponseBuilder::buildError(int errorPageCode) const {
	std::string bodyContent;
	std::map<std::string, std::string> errorHeaders;
	errorHeaders["Content-Type"] = "text/html";

	// 에러 코드별 Reason
	std::string errorReason = getReasonPhrase(errorPageCode);

	// 1. 커스텀 에러 페이지
	std::map<int, std::string>::const_iterator it = currConf_.errorPages_.find(errorPageCode);
	if (it != currConf_.errorPages_.end()) {
		const std::string &customFilePath = it->second;
		// 커스텀 페이지 로딩 성공 시 bodyContent에 채움
		readFileContentToString(customFilePath, bodyContent);
	}

	// 2. 기본 에러 페이지 디렉토리
	if (bodyContent.empty()) {
		std::ostringstream defaultPath;
		defaultPath << "./default_error_pages/error" << errorPageCode << ".html";
		readFileContentToString(defaultPath.str(), bodyContent);
	}

	// 3. 두 경우 모두 실패하면 fallback
	if (bodyContent.empty()) {
		bodyContent = "<html><head><title>Error</title></head>"
					  "<body><h1>Error</h1><p>Error page not available.</p></body></html>";
	}

	// Content-Length
	{
		std::ostringstream oss;
		oss << bodyContent.size();
		errorHeaders["Content-Length"] = oss.str();
	}

	return assembleResponse(errorPageCode, errorReason, errorHeaders, bodyContent);
}
