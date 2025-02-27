#ifndef CGI_HANDLER_HPP
#define CGI_HANDLER_HPP

#include "../ResponseBuilder/ResponseBuilder.hpp"      // 응답 빌더 클래스 선언 포함
#include "../ClientSession/ClientSession.hpp"        // 클라이언트 세션 클래스 선언 포함
#include <string>                   // 문자열 사용을 위한 헤더 포함
#include <vector>                   // 벡터 사용을 위한 헤더 포함

// Parsed URI components 구조체: CGI 스크립트 경로, 추가 경로 정보, 쿼리 문자열 저장
struct UriParts {
    std::string scriptPath;   // 예: "/var/www/html/cgi-bin/test.py"
    std::string pathInfo;     // 예: "/extra/path"
    std::string queryString;  // 예: "key=value"
};

// CgiHandler 클래스: CGI 요청 처리를 위한 클래스
class CgiHandler {
public:
    // 생성자: 응답 빌더를 받아 초기화
    explicit CgiHandler(const ResponseBuilder& responseBuilder);
    // 소멸자
    ~CgiHandler();

    // 주어진 URI가 CGI 대상인지 확인하는 함수
    bool isCGI(const std::string& targetUri, const std::string& cgiExtension);
    // 클라이언트 요청을 처리하여 CGI 실행 결과를 반환하는 함수
    std::string handleRequest(const ClientSession& clientSession);

private:
    const ResponseBuilder& responseBuilder_; // 응답 생성에 필요한 빌더 객체

    // CGI 실행에 필요한 환경 변수를 설정하는 함수
    void buildCgiEnv(const ClientSession& clientSession, 
                     const UriParts& uriParts,
                     std::vector<std::string>& envVars);

    // CGI 스크립트를 실행하고 결과를 반환하는 함수
    std::string executeCgi(const std::string& scriptPath,
                           const std::vector<std::string>& cgiEnv,
                           const std::string& requestBody);

    // 생성된 파이프들을 모두 닫는 함수
    void closePipes(int inPipe[2], int outPipe[2]);
    // 자식 프로세스에서 파이프를 표준 입출력으로 재설정하는 함수
    void setupChildPipes(int inPipe[2], int outPipe[2]);
    // 자식 프로세스에서 CGI 스크립트를 실행하는 함수
    void executeChild(const std::string& scriptPath, const std::vector<std::string>& cgiEnv);
    // 부모 프로세스에서 자식 프로세스를 관리하며 결과를 수신하는 함수
    std::string handleParent(pid_t pid, int inPipe[2], int outPipe[2], const std::string& requestBody);
    // "이름=값" 형태의 환경 변수 문자열을 생성하는 함수
    std::string makeEnvVar(const std::string& name, const std::string& value);
    // URI를 파싱하여 스크립트 경로, 추가 경로 정보, 쿼리 문자열을 추출하는 함수
    UriParts parseUri(const std::string& root, const std::string& uri);
};

#endif
