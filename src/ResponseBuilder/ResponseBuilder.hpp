#pragma once                                   // 중복 포함 방지

#include "GlobalConfig.hpp"                     // 전역 설정 헤더 포함
#include "ErrorPageResolver.hpp"                // 에러 페이지 해석기 헤더 포함
#include <map>                                  // std::map 사용
#include <string>                               // std::string 사용
#include <ctime>                                // 시간 관련 함수 사용
#include <sstream>                              // 문자열 스트림 사용
#include <iostream>                             // 입출력 스트림 사용

//  HTTP 응답을 조립하는 책임을 가진 클래스.
// 상태 코드/Reason, 헤더, 바디 등을 받아 최종 HTTP 응답 문자열을 생성한다.
// 일반 응답 생성 build()와 에러 응답 생성 buildError()를 제공한다.

class ResponseBuilder {
    public:
        explicit ResponseBuilder(const RequestConfig &conf); // 생성자: 요청 핸들링 설정을 전달받음
        ~ResponseBuilder();                                  // 소멸자

        // 응답 상태, 헤더, 바디를 설정하는 함수들
        void setStatus(int code, const std::string &reason); // 상태 코드와 이유를 설정
        void addHeader(const std::string &key, const std::string &value); // 헤더를 추가
        void setBody(const std::string &bodyContent);        // 응답 바디를 설정

        // 일반 응답 문자열을 생성하는 함수
        std::string build() const;                           // 최종 응답 문자열을 반환

        // 에러 응답 문자열을 생성하는 함수
        // (실제 에러 페이지 내용은 ErrorPageResolver가 결정 및 로드)
        std::string buildError(int errorPageCode) const;

    private:
        const RequestConfig&                currConf_;    // 현재 요청 핸들링 설정
        int                                 statusCode_;  // 응답 상태 코드
        std::string                         reasonPhrase_; // 상태 코드에 해당하는 이유 문구
        std::map<std::string, std::string>  headers_;     // 응답 헤더들을 저장하는 맵
        std::string                         body_;        // 응답 바디 내용

        // 내부에서 HTTP 응답 문자열을 조립하는 함수들
        std::string assembleResponse() const;              // 전체 응답 문자열을 조립

};
