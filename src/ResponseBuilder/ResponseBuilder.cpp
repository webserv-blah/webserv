#include "ResponseBuilder.hpp"	// ResponseBuilder 헤더 파일 포함
#include "../include/commonEnums.hpp"

// 현재 GMT 시간의 RFC1123 형식 문자열 반환
static std::string getCurrentDateString() {
	// 현재 시간 가져오기
	std::time_t now = std::time(NULL);
	// 날짜 문자열 저장할 버퍼 선언
	char buf[100];
	// GMT 기준 날짜 포맷으로 변환
	std::strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S GMT", std::gmtime(&now));
	// 변환된 문자열 반환
	return std::string(buf);
}

void ResponseBuilder::setContentLength(std::map<std::string, std::string>& headers, const std::string &body) const {
	// 바디 길이를 기반으로 Content-Length 헤더 설정
	std::ostringstream oss;	// 문자열 스트림 생성
	oss << body.size();		// 바디 길이를 문자열로 변환
	headers["Content-Length"] = oss.str();	// Content-Length 헤더에 값 설정
}

std::string ResponseBuilder::getReasonPhrase(int errorCode) const {
	// 상태 코드에 따른 이유 구문 반환
	switch (errorCode) {
		case OK: return "OK";                     // 200: OK
		case MOVED_PERMANENTLY: return "Moved Permanently";				// 301: 영구적으로 이동됨
		case FOUND: return "Found";                  					// 302: 찾음 (임시 이동)
		case BAD_REQUEST: return "Bad Request";            				// 400: 잘못된 요청
		case FORBIDDEN: return "Forbidden";              				// 403: 접근 거부
		case NOT_FOUND: return "Not Found";        						// 404: 찾을 수 없음
		case METHOD_NOT_ALLOWED: return "Method Not Allowed";   		// 405: 허용되지 않은 메소드
		case REQUEST_TIMEOUT: return "Request Timeout";        			// 408: 요청 시간 초과
		case CONTENT_TOO_LARGE: return "Payload Too Large";     		// 413: 요청 페이로드가 너무 큼
		case INTERNAL_SERVER_ERROR: return "Internal Server Error"; 	// 500: 내부 서버 오류
		case NOT_IMPLEMENTED: return "Not Implemented";					// 501: 구현되지 않음
		case SERVICE_UNAVAILABLE: return "Service Unavailable";			// 503: 서비스 이용 불가
		default:  return "Error";                  						// 그 외: 일반 오류
	}
}

std::string ResponseBuilder::assembleResponse(int statusCode, const std::string& reasonPhrase, 
	const std::map<std::string, std::string>& headers, const std::string& body) const {
	std::clog << "Start assemble Resp" << std::endl;
	// 최종 HTTP/1.1 응답 메시지 조립
	std::ostringstream oss;	// 응답 메시지 스트림 생성
	oss << "HTTP/1.1 " << statusCode << " " << reasonPhrase << "\r\n";	// 상태 라인 추가
	oss << "Date: " << getCurrentDateString() << "\r\n";	// Date 헤더 추가

	// 모든 헤더 추가
	for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it) {
		oss << it->first << ": " << it->second << "\r\n";	// 헤더 추가
	}

	oss << "\r\n";	// 헤더와 바디 구분을 위한 빈 줄 추가
	oss << body;	// 응답 바디 추가

	std::clog << "End assemble Resp" << std::endl;
	return oss.str();	// 완성된 응답 메시지 반환
}

std::string ResponseBuilder::build(	int statusCode, 
									std::map<std::string, std::string>& headers,  
									const std::string& body) const {
	// 일반 응답 메시지 생성
	std::string reason = getReasonPhrase(statusCode);	// 상태 코드에 따른 이유 구문 결정

	if (!body.empty()) {
		// Content-Length 값을 snprintf를 이용하여 변환
		char contentLengthBuffer[20];  // 충분한 크기의 버퍼 확보
		snprintf(contentLengthBuffer, sizeof(contentLengthBuffer), "%zu", body.size());
		headers["Content-Length"] = contentLengthBuffer;
	}

	return assembleResponse(statusCode, reason, headers, body);	// 최종 응답 메시지 반환
}

std::string ResponseBuilder::buildError(int errorStatusCode, const RequestConfig& currConf) const {
	// 에러 응답 메시지 생성
	std::string errorReason = getReasonPhrase(errorStatusCode);	// 에러 코드에 따른 이유 구문 결정
	std::clog << "[buildError] Done getReasonPhrase: " << errorReason << std::endl;
	std::map<std::string, std::string> headers;					// 헤더 저장용 맵
	std::string body = ErrorPageResolver::resolveErrorPage(errorStatusCode, currConf.errorPages_);	// 에러 페이지 HTML 내용 불러오기
	std::clog << "[buildError] Done resolveErrorPage: " << body << std::endl;

	headers["Content-Type"] = "text/html";	// Content-Type 헤더 설정
	setContentLength(headers, body);			// Content-Length 헤더 설정

	return assembleResponse(errorStatusCode, errorReason, headers, body);	// 에러 응답 메시지 반환
}
