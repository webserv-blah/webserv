#include "TestFramework.hpp"
#include "../../src/ResponseBuilder/ErrorPageResolver.hpp"
#include "../../src/include/commonEnums.hpp"
#include <map>
#include <string>

// 테스트 스위트 선언
TEST_SUITE(ErrorPageResolver)

// 기본 에러 페이지 테스트 (사용자 정의 에러 페이지 없을 때)
TEST_CASE(DefaultErrorPage, ErrorPageResolver) {
    // 비어있는 에러 페이지 맵 생성
    std::map<int, std::string> emptyErrorPages;
    
    // 404 에러 페이지 가져오기
    std::string errorPage = ErrorPageResolver::resolveErrorPage(NOT_FOUND, emptyErrorPages);
    
    // 기본 에러 페이지가 생성되었는지 확인
    ASSERT_FALSE(errorPage.empty());
    
    // 기본 에러 페이지에 필요한 내용이 포함되어 있는지 확인
    ASSERT_TRUE(errorPage.find("404") != std::string::npos);
    ASSERT_TRUE(errorPage.find("Not Found") != std::string::npos);
    ASSERT_TRUE(errorPage.find("The requested resource could not be found on this server") != std::string::npos);
}

// 사용자 정의 에러 페이지 테스트
TEST_CASE(CustomErrorPage, ErrorPageResolver) {
    // 테스트용 에러 페이지 맵 생성
    std::map<int, std::string> customErrorPages;
    // 절대 경로를 사용하여 HTML 파일에 접근
    customErrorPages[404] = "/Users/seonseo/projects/webserv/html/error/default_error.html";
    
    // 404 에러 페이지 가져오기
    std::string errorPage = ErrorPageResolver::resolveErrorPage(NOT_FOUND, customErrorPages);
    
    // 에러 페이지가 생성되었는지 확인
    ASSERT_FALSE(errorPage.empty());
    
    // 사용자 정의 에러 페이지가 제대로 로드되는지 확인
    // HTML 파일을 로드한 것이므로 HTML 태그가 있어야 함
    ASSERT_TRUE(errorPage.find("<html>") != std::string::npos);
    ASSERT_TRUE(errorPage.find("<head>") != std::string::npos);
    // 템플릿의 플레이스홀더가 그대로 있는지 확인
    ASSERT_TRUE(errorPage.find("{{ERROR_CODE}}") != std::string::npos);
}

// 존재하지 않는 사용자 정의 에러 페이지 테스트 (기본 페이지로 폴백)
TEST_CASE(NonExistentCustomErrorPage, ErrorPageResolver) {
    // 테스트용 에러 페이지 맵 생성 (존재하지 않는 파일 경로)
    std::map<int, std::string> invalidErrorPages;
    invalidErrorPages[404] = "/non_existent_file.html";
    
    // 404 에러 페이지 가져오기
    std::string errorPage = ErrorPageResolver::resolveErrorPage(NOT_FOUND, invalidErrorPages);
    
    // 기본 에러 페이지가 사용되었는지 확인
    ASSERT_FALSE(errorPage.empty());
    ASSERT_TRUE(errorPage.find("404") != std::string::npos);
    ASSERT_TRUE(errorPage.find("Not Found") != std::string::npos);
}

// 다양한 에러 코드 처리 테스트
TEST_CASE(DifferentErrorCodes, ErrorPageResolver) {
    std::map<int, std::string> emptyErrorPages;
    
    // 400 에러 페이지 테스트
    std::string badRequestPage = ErrorPageResolver::resolveErrorPage(BAD_REQUEST, emptyErrorPages);
    ASSERT_TRUE(badRequestPage.find("400") != std::string::npos);
    ASSERT_TRUE(badRequestPage.find("Bad Request") != std::string::npos);
    
    // 403 에러 페이지 테스트
    std::string forbiddenPage = ErrorPageResolver::resolveErrorPage(FORBIDDEN, emptyErrorPages);
    ASSERT_TRUE(forbiddenPage.find("403") != std::string::npos);
    ASSERT_TRUE(forbiddenPage.find("Forbidden") != std::string::npos);
    
    // 500 에러 페이지 테스트
    std::string serverErrorPage = ErrorPageResolver::resolveErrorPage(INTERNAL_SERVER_ERROR, emptyErrorPages);
    ASSERT_TRUE(serverErrorPage.find("500") != std::string::npos);
    ASSERT_TRUE(serverErrorPage.find("Internal Server Error") != std::string::npos);
}

// 템플릿 플레이스홀더 대체 테스트
TEST_CASE(TemplatePlaceholders, ErrorPageResolver) {
    std::map<int, std::string> emptyErrorPages;
    
    // 404 에러 페이지 가져오기
    std::string errorPage = ErrorPageResolver::resolveErrorPage(NOT_FOUND, emptyErrorPages);
    
    // 모든 플레이스홀더가 대체되었는지 확인
    ASSERT_TRUE(errorPage.find("{{ERROR_CODE}}") == std::string::npos);
    ASSERT_TRUE(errorPage.find("{{ERROR_TEXT}}") == std::string::npos);
    ASSERT_TRUE(errorPage.find("{{ERROR_DESCRIPTION}}") == std::string::npos);
    
    // 실제 값으로 대체되었는지 확인
    ASSERT_TRUE(errorPage.find("404") != std::string::npos);
    ASSERT_TRUE(errorPage.find("Not Found") != std::string::npos);
    ASSERT_TRUE(errorPage.find("The requested resource could not be found on this server") != std::string::npos);
}