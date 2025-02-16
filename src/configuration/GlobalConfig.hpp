#ifndef GLOBALCONFIG_HPP
# define GLOBALCONFIG_HPP

# include "Optional.hpp"
# include <string>
# include <vector>
# include <map>

// ReqHandleConf는 요청 처리를 위한 설정을 나타냅니다.
class ReqHandleConf {
public:
    std::vector<std::string> methods;         // 예: GET, POST 등
    std::map<int, std::string> errorPages;    // 오류 코드와 오류 페이지 매핑
    std::string returnUrl;                    // 반환 또는 리디렉션할 URL
    int returnStatus;                         // 리디렉션/반환을 위한 HTTP 상태 코드
    std::string root;                         // 요청의 루트 디렉토리
    std::string indexFile;                    // 기본 제공 파일 (예: index.html)
    std::string uploadPath;                   // 업로드된 파일을 저장할 경로
    std::string cgiExtension;                 // CGI 스크립트를 식별하는 확장자
    Optional<size_t> clientMaxBodySize;       // 요청의 최대 허용 본문 크기
    Optional<bool> autoIndex;                 // 디렉토리 자동 색인화 여부

    // 기본 생성자는 멤버를 기본값으로 초기화합니다.
    ReqHandleConf()
    : returnStatus(0)
    {}
};

// LocationConfig는 특정 위치 블록의 구성을 나타냅니다.
class LocationConfig {
public:
    std::string path;                         // 위치 경로 (예: "/images")
    ReqHandleConf reqHandling;                // 이 위치의 요청 처리 구성
};

// ServerConfig는 특정 서버의 구성을 나타냅니다.
class ServerConfig {
public:
    std::string host;                         // 호스트 이름 또는 IP 주소
    unsigned int port;                        // 수신 대기할 포트 번호
    std::vector<std::string> serverNames;     // 서버 이름 별칭
    ReqHandleConf reqHandling;                // 이 서버의 기본 요청 처리
    std::vector<LocationConfig> locations;    // 위치별 구성 목록

    // 기본 생성자는 원하는 경우 포트를 일반적인 기본값으로 초기화할 수 있습니다.
    ServerConfig() : port(80) {}              // 포트를 80으로 기본 설정합니다.
};

// GlobalConfig는 웹 서버의 전체 구성을 나타냅니다.
class GlobalConfig {
public:
    std::vector<ServerConfig> servers;        // 서버 구성 목록
    void print();
};

#endif