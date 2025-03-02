#include "TestFramework.hpp"
#include "../../src/GlobalConfig/GlobalConfig.hpp"
#include "../../src/ResponseBuilder/ResponseBuilder.hpp"
#include "../../src/include/commonEnums.hpp"
#include <string>
#include <sstream>
#include <map>
#include <unistd.h>

// 테스트를 위한 모의 객체 생성
// 실제 EventHandler에 의존하지 않고 handleRedirection 메서드 기능을 재구현

// 모의 EventHandler 클래스
class MockEventHandler {
private:
    ResponseBuilder responseBuilder_;
    
public:
    std::string handleRedirection(const RequestConfig& conf) {
        // 리다이렉션 설정이 올바른 경우
        if (!conf.returnUrl_.empty() && 
            (conf.returnStatus_ == MOVED_PERMANENTLY || conf.returnStatus_ == FOUND)) {
            // 리다이렉트 응답
            std::map<std::string, std::string> headers;
            headers["Location"] = conf.returnUrl_;
            return responseBuilder_.build(conf.returnStatus_, headers, "");
        } else {
            // 그 외에는 에러 응답
            return responseBuilder_.buildError(conf.returnStatus_, conf);
        }
    }
};

// TestSuite 선언
TEST_SUITE(EventHandler)

// handleRedirection 메서드 테스트
TEST_CASE(HandleRedirection301, EventHandler) {
    // 테스트 오브젝트 생성
    MockEventHandler eventHandler;
    
    // 리다이렉션 설정 (301 MOVED_PERMANENTLY)
    RequestConfig config;
    config.returnStatus_ = MOVED_PERMANENTLY;
    config.returnUrl_ = "https://example.com/new-location";
    
    // 리다이렉션 핸들링
    std::string response = eventHandler.handleRedirection(config);
    
    // 검증: 응답에 상태 코드 포함
    ASSERT_TRUE(response.find("301") != std::string::npos);
    
    // 검증: 응답에 Location 헤더 포함
    ASSERT_TRUE(response.find("Location: https://example.com/new-location") != std::string::npos);
    
    // 검증: 응답이 비어있지 않음
    ASSERT_FALSE(response.empty());
}

// 302 리다이렉션 테스트
TEST_CASE(HandleRedirection302, EventHandler) {
    MockEventHandler eventHandler;
    
    // 리다이렉션 설정 (302 FOUND)
    RequestConfig config;
    config.returnStatus_ = FOUND;
    config.returnUrl_ = "https://example.com/temporary-redirect";
    
    // 리다이렉션 핸들링
    std::string response = eventHandler.handleRedirection(config);
    
    // 검증: 응답에 상태 코드 포함
    ASSERT_TRUE(response.find("302") != std::string::npos);
    
    // 검증: 응답에 Location 헤더 포함
    ASSERT_TRUE(response.find("Location: https://example.com/temporary-redirect") != std::string::npos);
    
    // 검증: 응답이 비어있지 않음
    ASSERT_FALSE(response.empty());
}

// 잘못된 설정 테스트 - 빈 URL
TEST_CASE(HandleRedirectionEmptyUrl, EventHandler) {
    MockEventHandler eventHandler;
    
    // 잘못된 리다이렉션 설정 - URL이 비어 있음
    RequestConfig config;
    config.returnStatus_ = FOUND;
    config.returnUrl_ = ""; // 빈 URL
    
    // 리다이렉션 핸들링
    std::string response = eventHandler.handleRedirection(config);
    
    // 검증: 에러 응답이 생성됨
    ASSERT_TRUE(response.find("302") != std::string::npos); // 상태 코드는 그대로 사용됨
    
    // 검증: Location 헤더가 없음
    ASSERT_TRUE(response.find("Location:") == std::string::npos);
}

// 잘못된 설정 테스트 - 유효하지 않은 상태 코드
TEST_CASE(HandleRedirectionInvalidStatus, EventHandler) {
    MockEventHandler eventHandler;
    
    // 잘못된 리다이렉션 설정 - 유효하지 않은 상태 코드
    RequestConfig config;
    config.returnStatus_ = NOT_FOUND; // 404는 리다이렉션 코드가 아님
    config.returnUrl_ = "https://example.com/redirect";
    
    // 리다이렉션 핸들링
    std::string response = eventHandler.handleRedirection(config);
    
    // 검증: 에러 응답이 생성됨
    ASSERT_TRUE(response.find("404") != std::string::npos);
    
    // 검증: Location 헤더가 없음
    ASSERT_TRUE(response.find("Location:") == std::string::npos);
}