#ifndef RESPONSE_BUILDER_HPP
#define RESPONSE_BUILDER_HPP

#include "../GlobalConfig/GlobalConfig.hpp"
#include "ErrorPageResolver.hpp"
#include "commonEnums.hpp"

#include <map>
#include <string>
#include <ctime>
#include <sstream>
#include <iostream>

// HTTP 응답을 조립하는 클래스.
// 상태 코드/Reason, 헤더, 바디 등을 받아 최종 HTTP 응답 문자열을 생성한다.
// 일반 응답 생성 build()와 에러 응답 생성 buildError()를 제공한다.
class ResponseBuilder {
public:
	// 일반 응답 문자열을 생성하는 함수
	// @param status       HTTP 상태 코드
	// @param body         응답 바디
	// @param headers	   응답 생성에 사용될 헤더들
	// @return 최종 HTTP 응답 문자열
	std::string build(int status, std::map<std::string, std::string>& headers, const std::string& body) const;

	// 에러 응답 문자열을 생성하는 함수
	// (실제 에러 페이지 내용은 ErrorPageResolver가 결정 및 로드)
	// @param statusCode   HTTP 상태 코드
	// @param currConf     현재 요청에 대한 설정
	// @return 최종 에러 응답 문자열
	std::string buildError(EnumStatusCode statusCode, const RequestConfig& currConf) const;

	// Cgi 스크립트로부터 나온 결과물에 헤더를 추가해주는 함수
	// @param cgiOutput    CGI 스크립트의 출력 결과
	// @return 헤더가 추가된 결과물
	std::string AddHeaderForCgi(const std::string &cgiOutput) const;

private:

	// Content-Length 헤더를 설정하는 헬퍼함수
	void setContentLength(std::map<std::string, std::string>& headers, 
		const std::string& body) const;
	
	// 상태 코드에 대응하는 Reason Phrase를 반환하는 헬퍼함수
	
	// 전체 HTTP 응답 문자열을 조립하는 헬퍼함수
	// @param statusCode    HTTP 상태 코드
	// @param reasonPhrase  상태 코드에 대응하는 Reason Phrase
	// @param headers       헤더 맵
	// @param body          응답 바디
	// @return 최종 HTTP 응답 문자열
	std::string assembleResponse(int statusCode,
								const std::string& reasonPhrase,
								const std::map<std::string, std::string>& headers,
								const std::string& body) const;

};

#endif
