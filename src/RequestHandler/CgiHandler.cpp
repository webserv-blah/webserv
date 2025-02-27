#include "CgiHandler.hpp"               // CgiHandler 클래스 선언 헤더 포함
#include "../ClientSession/ClientSession.hpp"            // 클라이언트 세션 관련 클래스 포함
#include "../utils/file_utils.hpp"               // 파일 관련 유틸리티 포함
#include "../RequestMessage/RequestMessage.hpp"           // 요청 메시지 클래스 포함
#include "../GlobalConfig/GlobalConfig.hpp"             // 전역 설정 정보 포함
#include "../ResponseBuilder/ResponseBuilder.hpp"          // 응답 빌더 클래스 포함

#include <sys/wait.h>                   // 프로세스 대기 관련 헤더 포함
#include <unistd.h>                     // POSIX API 관련 헤더 포함
#include <sstream>                      // 문자열 스트림 사용을 위한 헤더 포함
#include <string>                       // 문자열 사용을 위한 헤더 포함
#include <vector>                       // 벡터 사용을 위한 헤더 포함

using namespace FileUtilities;          // FileUtilities 네임스페이스 사용

// CgiHandler 생성자: 응답 빌더를 초기화함
CgiHandler::CgiHandler(const ResponseBuilder& responseBuilder) : responseBuilder_(responseBuilder) {
}

// CgiHandler 소멸자
CgiHandler::~CgiHandler() {
}

// 요청된 URI가 CGI 실행 대상인지 확인하는 함수
bool CgiHandler::isCGI(const std::string& targetUri, const std::string& cgiExtension)
{
	// targetUri가 cgiExtension으로 끝나면 true 반환
    if (targetUri.size() < cgiExtension.size()) {
		return false;
	}
    return targetUri.compare(targetUri.size() - cgiExtension.size(), cgiExtension.size(), cgiExtension) == 0;
}

// 클라이언트의 요청을 처리하여 CGI 결과를 반환하는 함수
std::string CgiHandler::handleRequest(const ClientSession& clientSession) {
    // 클라이언트 요청 메시지와 설정 정보 가져옴
    const RequestMessage& reqMsg = clientSession.getReqMsg();
    const RequestConfig& conf = clientSession.getConfig();

    // URI를 파싱하여 스크립트 경로, 쿼리 문자열 등 추출
    UriParts parts = parseUri(conf.root_, reqMsg.getTargetURI());
    // 추출한 스크립트 경로의 유효성을 검사함
    EnumValidationResult vr = validatePath(parts.scriptPath);
    if (vr != VALID_FILE) {
        // 파일이 없거나 경로가 올바르지 않으면 404 또는 403 에러 반환
        return (vr == FILE_NOT_FOUND || vr == PATH_NOT_FOUND) ? 
               responseBuilder_.buildError(NOT_FOUND, conf) : 
               responseBuilder_.buildError(FORBIDDEN, conf);
    }
    if (vr == VALID_FILE && !hasExecutePermission(parts.scriptPath)) {
        return responseBuilder_.buildError(FORBIDDEN, conf);
    }
    // POST 요청이면 요청 본문을, 아니면 빈 문자열을 사용
    std::string requestBody = (reqMsg.getMethod() == POST) ? reqMsg.getBody() : "";
    // CGI 실행에 필요한 환경 변수를 저장할 벡터 생성
    std::vector<std::string> cgiEnv;
    // 환경 변수 설정 함수 호출
    buildCgiEnv(clientSession, parts, cgiEnv);
    // CGI 스크립트를 실행하고 결과를 받아옴
    std::string cgiResult = executeCgi(parts.scriptPath, cgiEnv, requestBody);
    // 실행 결과가 없으면 500 에러, 있으면 결과 반환
    return cgiResult.empty() ? responseBuilder_.buildError(INTERNAL_SERVER_ERROR, conf) : cgiResult;
}

// CGI 실행에 필요한 환경 변수를 설정하는 함수
void CgiHandler::buildCgiEnv(const ClientSession& clientSession, 
                             const UriParts& uriParts,
                             std::vector<std::string>& envVars) {
    // HTTP 메서드를 문자열로 변환하기 위한 배열
    static const std::string methodStrings[] = {"NONE", "GET", "POST", "DELETE"};
    // 클라이언트 요청 메시지와 설정 정보를 가져옴
    const RequestMessage& reqMsg = clientSession.getReqMsg();
    const RequestConfig& conf = clientSession.getConfig();

    envVars.reserve(11);               // 아래 설정할 변수 수만큼 벡터 공간 예약
    envVars.push_back(makeEnvVar("GATEWAY_INTERFACE", "CGI/1.1"));  // CGI 인터페이스 버전 설정
    envVars.push_back(makeEnvVar("REQUEST_METHOD", methodStrings[static_cast<int>(reqMsg.getMethod())]));  // 요청 메서드 설정
    envVars.push_back(makeEnvVar("SCRIPT_NAME", uriParts.scriptPath.substr(conf.root_.length())));  // 스크립트 이름 설정 (루트 이후 경로)
    envVars.push_back(makeEnvVar("PATH_INFO", uriParts.pathInfo));      // 추가 경로 정보 설정
    envVars.push_back(makeEnvVar("QUERY_STRING", uriParts.queryString));  // 쿼리 문자열 설정
    envVars.push_back(makeEnvVar("REMOTE_ADDR", clientSession.getClientIP()));  // 클라이언트 IP 주소 설정
    std::string host = reqMsg.getMetaHost();   // 요청 헤더에서 호스트 정보 추출
    std::string::size_type colonPos = host.find(':');  // 호스트 문자열에서 ':' 위치 탐색
    if (colonPos != std::string::npos) {
        envVars.push_back(makeEnvVar("SERVER_NAME", host.substr(0, colonPos)));   // ':' 전까지를 서버 이름으로 설정
        envVars.push_back(makeEnvVar("SERVER_PORT", host.substr(colonPos + 1)));    // ':' 이후를 서버 포트로 설정
    } else {
        envVars.push_back(makeEnvVar("SERVER_NAME", host));    // 호스트 전체를 서버 이름으로 설정
        envVars.push_back(makeEnvVar("SERVER_PORT", "80"));      // 포트 정보가 없으면 기본 포트 80 사용
    }
    envVars.push_back(makeEnvVar("SERVER_PROTOCOL", "HTTP/1.1"));  // 서버 프로토콜 설정
    envVars.push_back(makeEnvVar("SERVER_SOFTWARE", "MyTinyWebServer/0.1"));  // 서버 소프트웨어 정보 설정
}

// CGI 스크립트를 실행하고 결과를 반환하는 함수
std::string CgiHandler::executeCgi(const std::string& scriptPath,
                                   const std::vector<std::string>& cgiEnv,
                                   const std::string& requestBody) {
    int inPipe[2] = {-1, -1}, outPipe[2] = {-1, -1};   // 입력 및 출력 파이프 초기화
    if (pipe(inPipe) == -1) {
		return "";   // 입력 파이프 생성 실패 시 빈 문자열 반환
	}
    if (pipe(outPipe) == -1) {           // 출력 파이프 생성 실패 시
        close(inPipe[0]); close(inPipe[1]);  // 입력 파이프 닫음
        return "";                     // 빈 문자열 반환
    }

    pid_t pid = fork();                // 자식 프로세스 생성
    if (pid < 0) {
        closePipes(inPipe, outPipe);   // fork 실패 시 파이프 닫고 빈 문자열 반환
        return "";
    }

    if (pid == 0) {                    // 자식 프로세스인 경우
        setupChildPipes(inPipe, outPipe);  // 파이프 설정
        executeChild(scriptPath, cgiEnv);    // CGI 스크립트 실행
    } else {                           // 부모 프로세스인 경우
        return handleParent(pid, inPipe, outPipe, requestBody);  // 자식 관리 및 결과 수신
    }
    return "";
}

// 생성된 파이프들을 모두 닫는 함수
void CgiHandler::closePipes(int inPipe[2], int outPipe[2]) {
    if (inPipe[0] != -1) close(inPipe[0]);  // 입력 파이프 읽기 닫기
    if (inPipe[1] != -1) close(inPipe[1]);  // 입력 파이프 쓰기 닫기
    if (outPipe[0] != -1) close(outPipe[0]);  // 출력 파이프 읽기 닫기
    if (outPipe[1] != -1) close(outPipe[1]);  // 출력 파이프 쓰기 닫기
}

// 자식 프로세스에서 파이프를 표준 입출력으로 재설정하는 함수
void CgiHandler::setupChildPipes(int inPipe[2], int outPipe[2]) {
    dup2(inPipe[0], STDIN_FILENO);      // 입력 파이프를 표준 입력으로 연결
    dup2(outPipe[1], STDOUT_FILENO);     // 출력 파이프를 표준 출력으로 연결
    close(inPipe[1]);                   // 사용하지 않는 입력 파이프 쓰기 닫기
    close(outPipe[0]);                  // 사용하지 않는 출력 파이프 읽기 닫기
}

// 자식 프로세스에서 CGI 스크립트를 실행하는 함수
void CgiHandler::executeChild(const std::string& scriptPath, const std::vector<std::string>& cgiEnv) {
    std::vector<char*> envp(cgiEnv.size() + 1, 0);   // 환경 변수를 char* 배열로 변환할 벡터 생성
    for (std::vector<std::string>::size_type i = 0; i < cgiEnv.size(); ++i) {
        envp[i] = const_cast<char*>(cgiEnv[i].c_str());  // 각 환경 변수를 char*로 변환
    }

    char* argv[2];                      // 스크립트 실행 인자 배열 생성
    argv[0] = const_cast<char*>(scriptPath.c_str());  // 실행할 스크립트 경로 설정
    argv[1] = 0;                        // 인자 배열 끝을 표시

    execve(scriptPath.c_str(), argv, &envp[0]);  // execve 호출로 CGI 스크립트 실행
    _exit(1);                           // 실행 실패 시 자식 프로세스 종료
}

// 부모 프로세스에서 자식 프로세스를 관리하며 CGI 실행 결과를 수신하는 함수
std::string CgiHandler::handleParent(pid_t pid, int inPipe[2], int outPipe[2], const std::string& requestBody) {
    close(inPipe[0]);   // 부모 프로세스에서 입력 파이프 읽기 닫기
    close(outPipe[1]);  // 부모 프로세스에서 출력 파이프 쓰기 닫기

    // 요청 본문이 있는 경우, 자식 프로세스로 전송
    if (!requestBody.empty()) {
        std::string::size_type total = requestBody.size();
        const char* data = requestBody.c_str();
        std::string::size_type written = 0;
        while (written < total) {
            ssize_t bytes = write(inPipe[1], data + written, total - written);  // 데이터를 파이프에 씀
            if (bytes <= 0) break;  // 쓰기 실패 시 루프 종료
            written += bytes;
        }
    }
    close(inPipe[1]);  // 입력 파이프 쓰기 닫기

    std::ostringstream oss;            // 자식 프로세스의 출력을 저장할 스트림 생성
    char buffer[4096];                 // 읽기 버퍼 생성
    ssize_t bytesRead;

	//TODO: 여기서 read가 오류일땐..?
    while ((bytesRead = read(outPipe[0], buffer, sizeof(buffer))) > 0) {
        oss.write(buffer, bytesRead);  // 버퍼에서 읽은 데이터를 스트림에 기록
    }
    close(outPipe[0]);                 // 출력 파이프 읽기 닫기

    int status;
    waitpid(pid, &status, 0);          // 자식 프로세스 종료 상태를 대기 및 수신
    // 자식 프로세스가 정상 종료하면 결과 문자열 반환, 그렇지 않으면 빈 문자열 반환
    return (WIFEXITED(status) && WEXITSTATUS(status) == 0) ? oss.str() : "";
}

// "이름=값" 형태의 환경 변수 문자열을 생성하는 함수
std::string CgiHandler::makeEnvVar(const std::string& name, const std::string& value) {
    return name + "=" + value;
}


//TODO: cgi path를 더 효율적으로 구현할 수 있음.
// URI를 파싱하여 스크립트 경로, 추가 경로 정보, 쿼리 문자열 등을 추출하는 함수
UriParts CgiHandler::parseUri(const std::string& root, const std::string& uri) {
    UriParts parts;                   // 결과를 저장할 UriParts 구조체
    std::string fullPath = root + uri;  // 루트 경로와 URI를 결합하여 전체 경로 생성

    std::string::size_type queryPos = fullPath.find('?');  // '?' 문자의 위치 탐색
    std::string path = (queryPos == std::string::npos) ? fullPath : fullPath.substr(0, queryPos);  // 경로와 쿼리 분리
    parts.queryString = (queryPos == std::string::npos) ? "" : fullPath.substr(queryPos + 1);  // 쿼리 문자열 저장

    // "/cgi-bin/" 경로를 찾아 CGI 스크립트 경로를 결정
    std::string::size_type cgiPos = path.find("/cgi-bin/");
    if (cgiPos == std::string::npos) {
        parts.scriptPath = path;      // "/cgi-bin/"이 없으면 전체 경로를 스크립트 경로로 사용
        return parts;
    }

    std::string::size_type nextSlash = path.find('/', cgiPos + 9);  // "/cgi-bin/" 이후의 첫 번째 '/' 위치 탐색
    if (nextSlash == std::string::npos) {
        parts.scriptPath = path;      // 추가 경로 정보가 없으면 전체 경로를 스크립트 경로로 사용
    } else {
        parts.scriptPath = path.substr(0, nextSlash);  // 스크립트 경로와 추가 경로 분리
        if (validatePath(parts.scriptPath) == VALID_FILE) {
            parts.pathInfo = path.substr(nextSlash);  // 스크립트 경로가 유효하면 추가 경로 정보 저장
        } else {
            parts.scriptPath = path;  // 유효하지 않으면 전체 경로를 스크립트 경로로 사용
        }
    }

    return parts;  // 파싱 결과 반환
}
