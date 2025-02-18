#pragma once

# include "Optional.hpp"
# include <string>
# include <vector>
# include <map>

// RequestConfig는 요청 처리를 위한 설정을 나타냅니다.
class RequestConfig {
public:
	RequestConfig() : returnStatus_(0) {}			// 기본 생성자는 멤버를 기본값으로 초기화합니다.

	std::vector<std::string>	methods_;			// 예: GET, POST 등
	std::map<int, std::string>	errorPages_;		// 오류 코드와 오류 페이지 매핑
	std::string					returnUrl_;			// 반환 또는 리디렉션할 URL
	int							returnStatus_;		// 리디렉션/반환을 위한 HTTP 상태 코드
	std::string					root_;				// 요청의 루트 디렉토리
	std::string					indexFile_;			// 기본 제공 파일 (예: index.html)
	std::string					uploadPath_;		// 업로드된 파일을 저장할 경로
	std::string					cgiExtension_;		// CGI 스크립트를 식별하는 확장자
	Optional<size_t>			clientMaxBodySize_;	// 요청의 최대 허용 본문 크기
	Optional<bool>				autoIndex_;			// 디렉토리 자동 색인화 여부
};

// LocationConfig는 특정 위치 블록의 구성을 나타냅니다.
class LocationConfig {
public:
	std::string					path_;				// 위치 경로 (예: "/images")
	RequestConfig				reqConfig_;		// 이 위치의 요청 처리 구성
};

// ServerConfig는 특정 서버의 구성을 나타냅니다.
class ServerConfig {
public:
	ServerConfig() : port_(80) {}					// 포트를 80으로 기본 설정합니다.

	std::string					host_;				// 호스트 이름 또는 IP 주소
	unsigned int				port_;				// 수신 대기할 포트 번호
	std::vector<std::string>	serverNames_;		// 서버 이름 별칭
	RequestConfig				reqConfig_;		// 이 서버의 기본 요청 처리
	std::vector<LocationConfig>	locations_;			// 위치별 구성 목록
};

// GlobalConfig는 웹 서버의 전체 구성을 나타냅니다.
class GlobalConfig {
public:
	typedef std::map<int, std::vector<ServerConfig*> > ListenFdToServers;

	std::vector<ServerConfig>	servers_;			// 서버 구성 목록
	ListenFdToServers			listenFdToServers_;	// 리스닝 소켓 디스크립터 별 서버 구성 목록

	// 리스닝 소켓 디스크립터, 도메인 이름, 대상 URL에 해당하는 요청 설정 찾기
	const RequestConfig*		findRequestConfig(const int listenFd, \
												const std::string& domainName, \
												const std::string& targetUrl) const;
	// 설정 정보 출력
	void						print();

private:
	// 도메인 이름에 해당하는 서버 설정 찾기
	const ServerConfig&			findServerConfig(const std::vector<ServerConfig*>& servers, \
												const std::string& domainName) const;
	// 대상 URI에 해당하는 location 설정 찾기
	const RequestConfig*		findLocationConfig(const ServerConfig& server, \
													const std::string& targetUrl) const;
};
