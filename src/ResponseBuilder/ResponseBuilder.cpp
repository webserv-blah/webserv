#include "ResponseBuilder.hpp"                     // ResponseBuilder 헤더 파일 포함

// RFC1123 형식으로 현재 GMT 시각 문자열을 반환하는 함수
static std::string getCurrentDateString() {
    std::time_t now = std::time(NULL);            // 현재 시간을 얻음
    char buf[100];                                // 날짜 문자열을 저장할 버퍼 선언
    std::strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S GMT", std::gmtime(&now)); // GMT 기준 포맷으로 시간 변환
    return std::string(buf);                      // 변환된 문자열을 반환
}

ResponseBuilder::ResponseBuilder(const RequestConfig& conf) : currConf_(conf), statusCode_(200), reasonPhrase_("OK") {
}

ResponseBuilder::~ResponseBuilder() {
}

void ResponseBuilder::setStatus(int code, const std::string &reason){
    statusCode_ = code;                           // 상태 코드를 설정
    reasonPhrase_ = reason;                       // 상태 코드에 해당하는 이유 문구를 설정
}

void ResponseBuilder::addHeader(const std::string &key, const std::string &value){
    headers_[key] = value;                        // 헤더 맵에 키와 값을 추가
}

void ResponseBuilder::setBody(const std::string &bodyContent){
    body_ = bodyContent;                          // 응답 바디 내용을 설정

    // 바디의 길이에 맞춰 Content-Length 헤더를 설정
    std::ostringstream oss;                       // 문자열 스트림 생성
    oss << body_.size();                          // 바디의 크기를 문자열로 변환
    headers_["Content-Length"] = oss.str();       // Content-Length 헤더에 변환된 값 설정
}

// 현재 상태코드, reasonPhrase_, headers_, body_를 기반으로 최종 HTTP/1.1 응답 메시지 문자열을 조립하는 함수
std::string ResponseBuilder::assembleResponse() const {
    std::ostringstream oss;                       // 응답 메시지를 조립할 문자열 스트림 생성
    oss << "HTTP/1.1 " << statusCode_ << " " << reasonPhrase_ << "\r\n"; // 상태라인 추가
    oss << "Date: " << getCurrentDateString() << "\r\n"; // 현재 날짜 헤더 추가

    // 설정된 모든 헤더들을 추가
    for (std::map<std::string, std::string>::const_iterator it = headers_.begin();
         it != headers_.end(); ++it)
    {
        oss << it->first << ": " << it->second << "\r\n"; // 각 헤더를 "키: 값" 형식으로 추가
    }

    oss << "\r\n";                                // 헤더와 바디 사이에 빈 줄 추가
    oss << body_;                                 // 응답 바디 내용을 추가

    return oss.str();                             // 완성된 응답 문자열 반환
}

// 일반 응답 문자열을 생성하는 함수
std::string ResponseBuilder::build() const {
    // 바디가 비어있고 Content-Length 헤더가 설정되지 않은 경우
    if (body_.empty() && headers_.find("Content-Length") == headers_.end()) {
        const_cast<ResponseBuilder*>(this)->headers_["Content-Length"] = "0"; // Content-Length를 "0"으로 설정 (const_cast 사용)
    }
    return assembleResponse();                    // 최종 응답 문자열을 반환
}


// 에러 응답 문자열을 생성하는 함수
// ErrorPageResolver를 사용해 에러 페이지 HTML을 로드한 후,
//  새로운 ResponseBuilder를 통해 에러 응답 문자열을 구성한다.
std::string ResponseBuilder::buildError(int errorPageCode) const {
    std::string errorReason = ErrorPageResolver::getReasonPhrase(errorPageCode); // 에러 코드에 따른 Reason Phrase 결정

    std::string bodyContent = ErrorPageResolver::resolveErrorPage(errorPageCode, currConf_.errorPages_); // 에러 페이지 HTML 내용을 로드

    ResponseBuilder errorBuilder(currConf_);      // 현재 설정을 기반으로 새로운 ResponseBuilder 생성
    errorBuilder.setStatus(errorPageCode, errorReason); // 에러 상태 코드와 이유 설정
    errorBuilder.addHeader("Content-Type", "text/html");  // Content-Type 헤더를 "text/html"로 설정
    errorBuilder.setBody(bodyContent);              // 에러 페이지 내용을 바디에 설정

    return errorBuilder.build();                    // 생성된 에러 응답 문자열 반환
}
