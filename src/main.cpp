#include <iostream>
#include "utils.hpp"
#include "GlobalConfig.hpp"
//#include "ServerManager.hpp"

volatile bool globalServerRunning = true;
#include "RequestParser.hpp"
#ifndef BUFFER_SIZE
# define BUFFER_SIZE 8192 //== 8KB
#endif
EnumSesStatus recvRequest(ClientSession &curSession);
int main(int argc, char** argv) {
    if (argc != 2) {
		std::cerr << "Usage: ./webserver [config file path]" << std::endl;
		return 1;
	}
	GlobalConfig::initGlobalConfig(argv[1]);
	//setupSignalHandlers();
	//ServerManager serverManager;
	//serverManager.setupListeningSockets();
	//serverManager.run();
	//return 0;
	ClientSession ses(0, 0, "");
	std::cout << "================== START ==================" << std::endl;
	EnumSesStatus sessionStatus = recvRequest(ses);
	std::cout << "================== RESULT =================" << std::endl;
	std::cout << "(STATUSCODE: " << ses.getErrorStatusCode() << ")&";
	if (sessionStatus == REQUEST_ERROR)
		std::cout << "(REQUEST_ERROR)" << std::endl;
	if (sessionStatus == READ_CONTINUE)
		std::cout << "(READ_CONTINUE)" << std::endl;
	if (sessionStatus == READ_COMPLETE)
		std::cout << "(READ_COMPLETE)" << std::endl;
	ses.getReqMsg()->printResult();
	std::cout << "=================== END ===================" << std::endl;
}

EnumSesStatus recvRequest(ClientSession &curSession) {	
	// recv()대신 Request Message를 읽어옴
	const char* env = std::getenv("HTTP_REQUEST");
	if (env == NULL) {
		std::cerr << "환경변수 HTTP_REQUEST가 설정되지 않았습니다.\n";
		exit(1);
	}

	/* BUFFER_SIZE만큼 쪼개서 parsing하는 테스트
	*/
	std::string data(env);
	data.push_back('\n');
	RequestParser parser;
	std::cout << "ORIGIN ENV: "<<std::endl;
	for (std::string::const_iterator it = data.begin(); it != data.end(); ++it) {
		if (*it == '\n')
			std::cout << "\\n" << std::endl;
		else if (*it == '\r')
			std::cout << "\\r";
		else
			std::cout << *it;
	}
	std::cout <<";"<<std::endl;
	
	size_t dataLength = data.length();
	size_t offset = 0;
	EnumSesStatus requestResult = READ_CONTINUE;
	while (offset < dataLength) {
		size_t chunkSize = std::min(static_cast<size_t>(BUFFER_SIZE), dataLength - offset);
		std::string chunk(&data[offset], chunkSize);  // BUFFER_SIZE만큼 잘라서 문자열 생성

		const RequestConfig *config = curSession.getConfig();
		const size_t bodyMax = (config == NULL) ? BODY_MAX_LENGTH : config->clientMaxBodySize_.value();
		parser.setConfigBodyLength(bodyMax);

		// 이전에 버퍼 저장해놓은 데이터와 새로운 요청 데이터를 가지고 파싱
		EnumStatusCode statusCode = parser.parse(data, curSession);
		curSession.setErrorStatusCode(statusCode);

		EnumSesStatus requestResult;
		// 파싱이 끝나고 나서, 에러 status code와 RequestMessage의 상태를 점검함
		if (curSession.getErrorStatusCode() != NONE_STATUS_CODE)
			requestResult = REQUEST_ERROR;
		else if (curSession.getReqMsg()->getStatus() == REQ_DONE)
			requestResult = READ_COMPLETE;
		requestResult = READ_CONTINUE;

		if (requestResult == READ_COMPLETE) {
			std::cerr << "요청 처리 성공\n";
			break;
		}
		if (requestResult != READ_CONTINUE) {
			std::cerr << "요청 처리 실패\n";
			break;
		}
		offset += chunkSize;
	}

	return requestResult;
	/* BUFFER_SIZE가 적용되지 않은 테스트
	EnumSesStatus requestResult = curSession.implementReqMsg(parser, std::string(data));
	//if (requestResult == READ_COMPLETE)
		//handleAndResponse();
	return requestResult;
	*/
}