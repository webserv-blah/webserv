#include "CgiHandler.hpp"

#include "file_utils.hpp"
#include "RequestMessage.hpp"
#include "GlobalConfig.hpp"
#include "ResponseBuilder.hpp"

using namespace FileUtilities; // for validatePath()

CgiHandler::CgiHandler(const ResponseBuilder& responseBuilder)
    : responseBuilder_(responseBuilder)
{
}

CgiHandler::~CgiHandler()
{
}

// main: CGI 스크립트 경로 파싱, 권한 검사 -> execve -> 결과 문자열 or 에러문자열
std::string CgiHandler::handleRequest(const RequestMessage& reqMsg, const RequestConfig& conf)
{
    // 예: uri = "/cgi-bin/test.py?name=foo"
    // 실제 스크립트 경로: conf.root_ + "/cgi-bin/test.py"
    // 여기서는 간단히 query string을 떼어낸다
    std::string uri = reqMsg.getTargetURI();
    std::string fullPath = conf.root_ + uri;  // ex) "/var/www/html/cgi-bin/test.py?..."

    // '?' 이전까지가 실제 파일 경로
    std::size_t qPos = fullPath.find('?');
    std::string scriptPath = (qPos == std::string::npos)
                               ? fullPath
                               : fullPath.substr(0, qPos);

    // 파일/권한 검사
    EnumValidationResult vr = validatePath(scriptPath);
    if (vr != VALID_FILE) {
        // 파일이 없거나 디렉토리인 경우 -> 404
        if (vr == FILE_NOT_FOUND || vr == PATH_NOT_FOUND) {
            return responseBuilder_.buildError(404, conf);
        }
        // 파일은 있는데 권한이 없으면 -> 403
        return responseBuilder_.buildError(403, conf);
    }

    // 요청 본문 (POST라면 내용이 있을 수 있음)
    std::string requestBody;
    if (reqMsg.getMethod() == "POST") {
        requestBody = reqMsg.getBody();
    }

    // CGI 환경 변수 구성
    std::vector<std::string> cgiEnv = buildCgiEnv(reqMsg, scriptPath, conf);

    // 실제 CGI 실행 -> raw output
    std::string cgiResult = executeCgi(scriptPath, cgiEnv, requestBody);

    // execve 수행 도중 실패하거나, 자식이 비정상 종료한 경우 -> cgiResult == ""
    if (cgiResult.empty()) {
        return responseBuilder_.buildError(500, conf); // 내부 서버 에러
    }

    // 정상적으로 CGI가 출력한 문자열
    // (HTTP 헤더/바디 형태인지, 혹은 단순 text인지 여부 관계없이 그대로 반환)
    return cgiResult;
}

// CGI 환경 변수 구성
std::vector<std::string> CgiHandler::buildCgiEnv(const RequestMessage& reqMsg,
                                                 const std::string& scriptPath,
                                                 const RequestConfig& conf)
{
    std::vector<std::string> envVars;

    // REQUEST_METHOD
    {
        std::string method = reqMsg.getMethod();
        envVars.push_back("REQUEST_METHOD=" + method);
    }

    // SCRIPT_FILENAME (혹은 SCRIPT_NAME)
    {
        envVars.push_back("SCRIPT_FILENAME=" + scriptPath);
    }

    // QUERY_STRING
    {
        const std::string& uri = reqMsg.getTargetURI();
        std::size_t pos = uri.find('?');
        std::string query = (pos == std::string::npos) ? "" : uri.substr(pos + 1);
        envVars.push_back("QUERY_STRING=" + query);
    }

    // POST라면 CONTENT_LENGTH, CONTENT_TYPE
    if (reqMsg.getMethod() == "POST") {
        {
            std::ostringstream oss;
            oss << reqMsg.getBody().size();
            envVars.push_back("CONTENT_LENGTH=" + oss.str());
        }
        // 실제 프로젝트에선 RequestMessage에서 Content-Type 헤더를 가져옴
        // 여기선 기본값만 넣어두기
        envVars.push_back("CONTENT_TYPE=application/x-www-form-urlencoded");
    }

    // 표준 CGI 변수들 (예시)
    envVars.push_back("GATEWAY_INTERFACE=CGI/1.1");
    envVars.push_back("SERVER_PROTOCOL=HTTP/1.1");
    envVars.push_back("SERVER_SOFTWARE=MyTinyWebServer/0.1");

    // 추가로 필요하다면 HOST, SERVER_NAME, REMOTE_ADDR 등 설정 가능

    return envVars;
}

// fork/execve -> 자식 STDIN에 requestBody 써주고, 자식 STDOUT 전부 수집
std::string CgiHandler::executeCgi(const std::string& scriptPath,
                                   const std::vector<std::string>& cgiEnv,
                                   const std::string& bodyForCgi) {
    // 파이프 2개: 부모->자식(stdin), 자식->부모(stdout)
    int inPipe[2];
    int outPipe[2];
    if (pipe(inPipe) == -1 || pipe(outPipe) == -1) {
        return "";
    }

    pid_t pid = fork();
    if (pid < 0) {
        // fork 실패
        close(inPipe[0]); close(inPipe[1]);
        close(outPipe[0]); close(outPipe[1]);
        return "";
    }

    if (pid == 0) {
        // 자식 프로세스
        // 파이프 재배치
        dup2(inPipe[0], STDIN_FILENO);
        dup2(outPipe[1], STDOUT_FILENO);

        // 안쓰는 파이프 fd 닫기
        close(inPipe[1]);
        close(outPipe[0]);

        // envp 구성
        std::vector<char*> envp;
        envp.reserve(cgiEnv.size() + 1);
        for (std::size_t i = 0; i < cgiEnv.size(); i++) {
            envp.push_back(const_cast<char*>(cgiEnv[i].c_str()));
        }
        envp.push_back(NULL);

        // argv 구성 (스크립트 경로 하나만)
        std::vector<char*> argv;
        argv.push_back(const_cast<char*>(scriptPath.c_str()));
        argv.push_back(NULL);

        // execve
        execve(scriptPath.c_str(), &argv[0], &envp[0]);

        // 실패 시 종료
        _exit(1);

    } else {
        // 부모 프로세스
        close(inPipe[0]);
        close(outPipe[1]);

        // CGI의 STDIN에 요청 바디 쓰기 (POST일 때)
        if (!bodyForCgi.empty()) {
            ssize_t written = write(inPipe[1], bodyForCgi.data(), bodyForCgi.size());
            (void)written; // 에러 처리는 간단히 생략
        }
        close(inPipe[1]);

        // 자식 STDOUT 읽기
        std::ostringstream oss;
        char buffer[4096];
        while (true) {
            ssize_t r = read(outPipe[0], buffer, sizeof(buffer));
            if (r <= 0) {
                break;
            }
            oss.write(buffer, r);
        }
        close(outPipe[0]);

        // 자식 종료 대기
        int status = 0;
        waitpid(pid, &status, 0);
        if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
            // 자식 비정상 종료 or exit code != 0 -> 실패 처리
            return "";
        }

        // CGI 결과(자식 stdout)
        return oss.str();
    }

    // 이 위치에는 도달하지 않음
    return "";
}
