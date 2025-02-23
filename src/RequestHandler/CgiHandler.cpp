#include "CgiHandler.hpp"
#include "ClientSession.hpp"
#include "file_utils.hpp"
#include "RequestMessage.hpp"
#include "GlobalConfig.hpp"
#include "ResponseBuilder.hpp"

using namespace FileUtilities; // for validatePath()

CgiHandler::CgiHandler(const ResponseBuilder& responseBuilder)
	: responseBuilder_(responseBuilder) {
}

CgiHandler::~CgiHandler() {
}

// CGI 스크립트 경로 파싱, 권한 검사 -> execve -> 결과 문자열 or 에러문자열
std::string CgiHandler::handleRequest(const ClientSession& clientSession) {
	// 예: uri = "/cgi-bin/test.py?name=foo"
	// 실제 스크립트 경로: conf.root_ + "/cgi-bin/test.py"
	// 여기서는 간단히 query string을 떼어낸다
	const RequestMessage& reqMsg = clientSession.getReqMsg();   //TODO: ClientSession의 RequestMessage와 RequestConfig 반환타입을 
	const RequestConfig& conf = clientSession.getConfig();      //      const RequestMessage& const RequestConfig&로 요청드리기..
	std::string uri = reqMsg.getTargetURI();
	std::string fullPath = conf.root_ + uri;  // ex) "/var/www/html/cgi-bin/test.py?..."

	// '?' 이전까지가 실제 파일 경로
	std::size_t queryPos = fullPath.find('?');
	std::string scriptPath = (queryPos == std::string::npos) ? fullPath : fullPath.substr(0, queryPos);

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
	if (reqMsg.getMethod() == POST) {
		requestBody = reqMsg.getBody();
	}

	// CGI 환경 변수 구성
	std::vector<std::string> cgiEnv;
	buildCgiEnv(clientSession, cgiEnv);

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

// 헬퍼 함수: 환경변수 이름과 값을 결합
std::string makeEnvVar(const std::string& name, const std::string& value) {
	return name + "=" + value;
}

std::string getPathInfo() {
	
}

// CGI 환경 변수 구성
void CgiHandler::buildCgiEnv(const ClientSession& clientSession, std::vector<std::string>& envVars) {
    envVars.reserve(10);
    static const std::string methodStrings[] = {"NONE", "GET", "POST", "DELETE"};

    envVars.push_back(makeEnvVar("GATEWAY_INTERFACE", "CGI/1.1"));

    const std::string& uri = reqMsg.getTargetURI();
    std::string::size_type queryPos = uri.find('?');
    std::string scriptName = (queryPos == std::string::npos) ? uri : uri.substr(0, queryPos);
    std::string queryString = (queryPos == std::string::npos) ? "" : uri.substr(queryPos + 1);
    std::string pathInfo = "";

    envVars.push_back(makeEnvVar("PATH_INFO", pathInfo));
    envVars.push_back(makeEnvVar("QUERY_STRING", queryString));
    envVars.push_back(makeEnvVar("SCRIPT_NAME", scriptName));

    envVars.push_back(makeEnvVar("REQUEST_METHOD", 
                                 methodStrings[static_cast<int>(reqMsg.getMethod())]));

    envVars.push_back(makeEnvVar("SERVER_NAME", "localhost"));
    envVars.push_back(makeEnvVar("SERVER_PORT", "80"));
    envVars.push_back(makeEnvVar("SERVER_PROTOCOL", "HTTP/1.1"));
    envVars.push_back(makeEnvVar("SERVER_SOFTWARE", "MyTinyWebServer/0.1"));
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
