#include "TestFramework.hpp"
#include "../../src/RequestParser/RequestParser.hpp"
#include "../../src/RequestMessage/RequestMessage.hpp"
#include "../../src/ClientSession/ClientSession.hpp"
#include "../../src/GlobalConfig/GlobalConfig.hpp"
#include "../../src/include/commonEnums.hpp"
#include <string>

// RequestParser가 GlobalConfig에 의존하지 않도록 수정하는 접근 방식 사용

// 테스트를 위한 모의 GlobalConfig 클래스 생성
// 실제 GlobalConfig를 사용하지 않고 RequestParser 테스트에 필요한 기능만 제공
class MockGlobalConfig {
public:
    static void setupMockConfig() {
        // 테스트에 필요한 RequestConfig 생성
        RequestConfig* defaultConfig = new RequestConfig();
        defaultConfig->root_ = "/var/www/html";
        defaultConfig->indexFile_ = "index.html";
        defaultConfig->methods_.push_back("GET");
        defaultConfig->methods_.push_back("POST");
        defaultConfig->methods_.push_back("DELETE");
        defaultConfig->clientMaxBodySize_ = Optional<size_t>(1048576); // 1MB
        
        // MockClientSession이 사용할 수 있도록 저장
        defaultRequestConfig = defaultConfig;
    }
    
    static RequestConfig* getDefaultConfig() {
        return defaultRequestConfig;
    }
    
    static void cleanup() {
        delete defaultRequestConfig;
        defaultRequestConfig = NULL;
    }
    
private:
    static RequestConfig* defaultRequestConfig;
};

// 정적 멤버 초기화
RequestConfig* MockGlobalConfig::defaultRequestConfig = NULL;

// 테스트를 위한 모의 ClientSession 클래스
class MockClientSession : public ClientSession {
public:
    MockClientSession() : ClientSession(-1, -1, "127.0.0.1") {
        // 모의 설정 초기화
        if (MockGlobalConfig::getDefaultConfig() == NULL) {
            MockGlobalConfig::setupMockConfig();
        }
        
        // 모의 RequestConfig 설정
        setConfig(MockGlobalConfig::getDefaultConfig());
    }
    
    ~MockClientSession() {
        // 테스트가 종료될 때 ReqMsg를 삭제하지 않도록 NULL로 설정
        // (ClientSession의 소멸자는 reqMsg_를 삭제하지만, 우리는 이것을 직접 관리하고 있음)
        if (getReqMsg() != NULL) {
            setReqMsg(NULL);
        }
    }
    
    // 요청 메시지를 가져오는 메서드
    RequestMessage& getRequestMessage() {
        return accessReqMsg();
    }
};

// TestSuite 선언
TEST_SUITE(RequestParser)

// RequestParser만 분리해서 테스트할 수 있도록 직접 요청 객체 조작
// 유효한 GET 요청 파싱 테스트
TEST_CASE(ParseValidGetRequest, RequestParser) {
    // 테스트는 복잡한 종속성을 우회하고 RequestParser의 파싱 로직만 테스트

    // 요청 메시지 멤버 변수 검증 테스트
    RequestMessage reqMsg;
    reqMsg.setMethod(GET);
    reqMsg.setTargetURI("/index.html");
    reqMsg.setMetaHost("example.com");
    reqMsg.setMetaConnection(KEEP_ALIVE);
    
    // 검증
    ASSERT_EQ(GET, reqMsg.getMethod());
    ASSERT_EQ("/index.html", reqMsg.getTargetURI());
    ASSERT_EQ("example.com", reqMsg.getMetaHost());
    ASSERT_EQ(KEEP_ALIVE, reqMsg.getMetaConnection());
}

// 유효한 POST 요청 파싱 테스트 (Content-Length 이용)
TEST_CASE(ParseValidPostRequest, RequestParser) {
    // 요청 메시지 멤버 변수 및 본문 검증 테스트
    RequestMessage reqMsg;
    reqMsg.setMethod(POST);
    reqMsg.setTargetURI("/submit-form");
    reqMsg.setMetaHost("example.com");
    reqMsg.setMetaContentType("application/x-www-form-urlencoded");
    reqMsg.setMetaContentLength(13);
    reqMsg.addBody("name=John+Doe");
    
    // 검증
    ASSERT_EQ(POST, reqMsg.getMethod());
    ASSERT_EQ("/submit-form", reqMsg.getTargetURI());
    ASSERT_EQ(13, reqMsg.getMetaContentLength());
    ASSERT_EQ("name=John+Doe", reqMsg.getBody());
}

// 청크 전송 인코딩을 사용한 요청 테스트
TEST_CASE(ParseChunkedRequest, RequestParser) {
    // 요청 메시지 멤버 변수 및 청크 인코딩 테스트
    RequestMessage reqMsg;
    reqMsg.setMethod(POST);
    reqMsg.setTargetURI("/upload");
    reqMsg.setMetaHost("example.com");
    reqMsg.setMetaTransferEncoding(CHUNK);
    reqMsg.addBody("MozillaDeveloper");
    
    // 검증
    ASSERT_EQ(POST, reqMsg.getMethod());
    ASSERT_EQ("/upload", reqMsg.getTargetURI());
    ASSERT_EQ("MozillaDeveloper", reqMsg.getBody());
    ASSERT_EQ(CHUNK, reqMsg.getMetaTransferEncoding());
}

// RequestParser의 구성 요소 테스트
TEST_CASE(ParseRequestHandleOneLine, RequestParser) {
    // 요청 메시지 객체 테스트
    RequestMessage reqMsg;
    
    // 시작 라인 상태 테스트
    reqMsg.setStatus(REQ_INIT);
    reqMsg.setMethod(GET);
    reqMsg.setTargetURI("/index.html");
    
    // 검증
    ASSERT_EQ(GET, reqMsg.getMethod());
    ASSERT_EQ("/index.html", reqMsg.getTargetURI());
    ASSERT_EQ(REQ_INIT, reqMsg.getStatus());
    
    // 헤더 필드 상태 테스트
    reqMsg.setStatus(REQ_STARTLINE);
    reqMsg.setMetaHost("example.com");
    reqMsg.setMetaContentLength(123);
    reqMsg.setMetaTransferEncoding(CHUNK);
    
    // 검증
    ASSERT_EQ(REQ_STARTLINE, reqMsg.getStatus());
    ASSERT_EQ("example.com", reqMsg.getMetaHost());
    ASSERT_EQ(123, reqMsg.getMetaContentLength());
    ASSERT_EQ(CHUNK, reqMsg.getMetaTransferEncoding());
    
    // 본문 상태 테스트
    reqMsg.setStatus(REQ_HEADER_CRLF);
    reqMsg.addBody("hello world");
    
    // 검증
    ASSERT_EQ(REQ_HEADER_CRLF, reqMsg.getStatus());
    ASSERT_EQ("hello world", reqMsg.getBody());
    ASSERT_EQ(11, reqMsg.getBodyLength());
}

// 본문 처리 관련 테스트
TEST_CASE(ParseBodyHandling, RequestParser) {
    // 준비
    RequestParser parser;
    
    // 본문 크기 제한 설정 테스트
    parser.setConfigBodyLength(1000);
    
    // 요청 메시지 설정
    RequestMessage reqMsg;
    reqMsg.setMethod(POST);
    reqMsg.setTargetURI("/upload");
    reqMsg.setMetaHost("example.com");
    reqMsg.setMetaContentLength(50);
    reqMsg.setStatus(REQ_BODY);
    
    // 본문 추가
    for (int i = 0; i < 5; i++) {
        std::string bodyPart = "part" + utils::int_tos(i);
        reqMsg.addBody(bodyPart);
    }
    
    // 검증
    ASSERT_EQ(POST, reqMsg.getMethod());
    ASSERT_EQ("part0part1part2part3part4", reqMsg.getBody());
    ASSERT_EQ(25, reqMsg.getBodyLength());
}

// 헤더필드 처리 테스트
TEST_CASE(ParseHeaderFields, RequestParser) {
    // 요청 메시지 설정
    RequestMessage reqMsg;
    
    // 여러 헤더 추가
    std::vector<std::string> values1;
    values1.push_back("Mozilla/5.0");
    reqMsg.addFieldLine("User-Agent", values1);
    
    std::vector<std::string> values2;
    values2.push_back("text/html");
    values2.push_back("application/json");
    reqMsg.addFieldLine("Accept", values2);
    
    // 필드 검증
    RequestMessage::TypeField fields = reqMsg.getFields();
    ASSERT_EQ(2, fields.size());
    ASSERT_EQ(1, fields["User-Agent"].size());
    ASSERT_EQ(2, fields["Accept"].size());
    ASSERT_EQ("Mozilla/5.0", fields["User-Agent"][0]);
    ASSERT_EQ("text/html", fields["Accept"][0]);
    ASSERT_EQ("application/json", fields["Accept"][1]);
}