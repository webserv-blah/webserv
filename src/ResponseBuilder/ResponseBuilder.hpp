#pragma once

#include "GlobalConfig.hpp"
#include <map>
#include <string>

class ResponseBuilder {
public:
	// 생성자/소멸자
	explicit ResponseBuilder(const ReqHandleConf &conf);
	~ResponseBuilder();

	// 응답 상태, 헤더, 바디 설정
	void setStatus(int code, const std::string &reason);
	void addHeader(const std::string &key, const std::string &value);
	void setBody(const std::string &bodyContent);

	// 일반 응답 생성
	std::string build();

	// 에러 응답 생성 (커스텀/기본/기본 Fallback)
	std::string buildError(int errorPageCode) const;

private:
	// 내부 헬퍼 메서드들
	std::string assembleResponse(int statusCode,
								 const std::string &reasonPhrase,
								 const std::map<std::string, std::string> &headers,
								 const std::string &body) const;

	// 상태코드 -> 기본 Reason-Phrase 매핑
	std::string getReasonPhrase(int code) const;

	// 현재 날짜(RFC1123)를 문자열로 반환
	std::string getCurrentDate() const;

	// 파일 내용 로딩 (커스텀 에러 페이지나 기본 에러 페이지 읽기에 활용)
	bool readFileContentToString(const std::string &filePath, std::string &outContent) const;

private:
	const ReqHandleConf&				currConf_;
	int									statusCode_;
	std::string							reasonPhrase_;
	std::map<std::string, std::string>	headers_;
	std::string							body_;
};
