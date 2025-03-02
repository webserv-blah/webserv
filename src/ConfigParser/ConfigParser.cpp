#include "ConfigParser.hpp"
#include "../include/errorUtils.hpp"

// globalConfig와 파일 경로를 받아 설정 파일을 파싱하는 함수
void ConfigParser::parse(GlobalConfig& globalConfig, const char* path) {
	// 파일 경로로부터 입력 스트림을 생성함
	std::ifstream configFile(path);
	if (!configFile.is_open()) {
		// 파일을 열지 못하면 예외 발생
		webserv::throwError("Failed to open config file", std::string(path), "ConfigParser::parse");
	}

	std::string token;
	// 파일의 끝까지 토큰을 읽음
	while (true) {
		token = getNextToken(configFile);
		if (token.empty())
			break;
		// "server" 토큰이 나오면 server 설정 블록을 파싱함
		if (token == "server") {
			ServerConfig server;
			parseServerBlock(configFile, server);
			// 같은 주소 포트 쌍을 가진 서버가 존재하면 에러
			if (isDuplicateServer(globalConfig.servers_, server)) {
				throw std::runtime_error("Duplicate server block found in config file");
			} else {
				globalConfig.servers_.push_back(server);
			}
		} else {
			// 예상하지 못한 토큰이면 예외 발생
			throw std::runtime_error("Unexpected token in config file: " + token);
		}
	}

	// server 블록이 없으면 예외 발생
    if (globalConfig.servers_.empty()) {
        throw std::runtime_error("No server blocks found in config file");
    }

	// 각 location 블록에 대해 server의 request handling을 적용
	for (std::vector<ServerConfig>::iterator server = globalConfig.servers_.begin(); server != globalConfig.servers_.end(); ++server) {
		for (std::vector<LocationConfig>::iterator location = server->locations_.begin(); location != server->locations_.end(); ++location) {
			getEffectiveReqHandling(server->reqConfig_, location->reqConfig_);
		}
	}
	// 각 request handling에 대해 기본값을 설정
	for (std::vector<ServerConfig>::iterator server = globalConfig.servers_.begin(); server != globalConfig.servers_.end(); ++server) {
		setDefaultReqHandling(server->reqConfig_);
		for (std::vector<LocationConfig>::iterator location = server->locations_.begin(); location != server->locations_.end(); ++location) {
			setDefaultReqHandling(location->reqConfig_);
		}
	}
}

// server 블록을 파싱하는 함수
void ConfigParser::parseServerBlock(std::ifstream& configFile, ServerConfig& serverBlock) {
	// server 블록 시작 '{'를 기대함
	std::string token = getNextToken(configFile);
	if (token != "{") {
		throw std::runtime_error("Expected '{' after server directive, got: " + token);
	}

	// '}'를 만날 때까지 토큰을 처리함
	while (true) {
		token = getNextToken(configFile);
		if (token.empty()) {
			// 파일 끝에 도달하면 예외 발생
			throw std::runtime_error("Unexpected end of file in server block");
		}
		// server 블록 종료 시
		if (token == "}") {
			break;
		}

		// 각 토큰에 따른 파싱 처리
		if (token == "listen") {
			parseHostPort(configFile, serverBlock.host_, serverBlock.port_);
		}
		else if (token == "server_name") {
			parseServerNames(configFile, serverBlock.serverNames_);
		}
		else if (token == "location") {
			serverBlock.locations_.push_back(LocationConfig());
			parseLocationBlock(configFile, serverBlock.locations_.back());
			continue;
		}
		else {
			// 그 외의 경우 request handling 관련 설정을 파싱함
			parseRequestConfig(configFile, serverBlock.reqConfig_, token);
		}

		// 각 지시문이 끝난 후 ';'가 있어야 함
		token = getNextToken(configFile);
		if (token != ";") {
			throw std::runtime_error("Expected ';' after a simple directive");
		}
	}
}

// location 블록을 파싱하는 함수
void ConfigParser::parseLocationBlock(std::ifstream& configFile, LocationConfig& locationBlock) {
	// location 경로 토큰을 파싱함
	parsePath(configFile, locationBlock.path_);

	// location 블록 시작 '{'를 기대함
	std::string token = getNextToken(configFile);
	if (token != "{") {
		throw std::runtime_error("Expected '{' after location directive");
	}

	// '}'를 만날 때까지 토큰을 처리함
	while (true) {
		token = getNextToken(configFile);
		if (token.empty()) {
			// 파일 끝에 도달하면 예외 발생
			throw std::runtime_error("Unexpected end of file in location block");
		}
		// location 블록 종료 시
		if (token == "}") {
			break;
		}

		if (token == "methods") {
			parseMethods(configFile, locationBlock.reqConfig_.methods_);
		} else {
			// request handling 관련 설정을 파싱함
			parseRequestConfig(configFile, locationBlock.reqConfig_, token);
		}

		// 각 지시문이 끝난 후 ';'가 있어야 함
		token = getNextToken(configFile);
		if (token != ";") {
			throw std::runtime_error("Expected ';' after a simple directive");
		}
	}
}

// request handling 설정을 파싱하는 함수
void ConfigParser::parseRequestConfig(std::ifstream& configFile, RequestConfig& reqConfig, const std::string& token) {
	if (token == "error_page") {
		parseErrorPage(configFile, reqConfig.errorPages_);
	} else if (token == "return") {
		parseReturn(configFile, reqConfig.returnUrl_, reqConfig.returnStatus_);
	} else if (token == "root") {
		parseRoot(configFile, reqConfig.root_);
	} else if (token == "index") {
		parseIndexFile(configFile, reqConfig.indexFile_);
	} else if (token == "upload_path") {
		parseUploadPath(configFile, reqConfig.uploadPath_);
	} else if (token == "cgi_extension") {
		parseCgiExtension(configFile, reqConfig.cgiExtension_);
	} else if (token == "client_max_body_size") {
		parseClientMaxBodySize(configFile, reqConfig.clientMaxBodySize_);
	} else if (token == "autoindex") {
		parseAutoIndex(configFile, reqConfig.autoIndex_);
	} else {
		// 예상하지 못한 토큰이면 예외 발생
		throw std::runtime_error("Unexpected token: " + token);
	}
}

// 서버 설정의 request handling을 location 설정에 적용하는 함수
void ConfigParser::getEffectiveReqHandling(const RequestConfig& serverReqHandling, RequestConfig& locationReqHandling) {
	if (locationReqHandling.methods_.empty()) {
		locationReqHandling.methods_ = serverReqHandling.methods_;
	}
	if (locationReqHandling.errorPages_.empty()) {
		locationReqHandling.errorPages_ = serverReqHandling.errorPages_;
	}
	if (locationReqHandling.returnUrl_.empty()) {
		locationReqHandling.returnUrl_ = serverReqHandling.returnUrl_;
	}
	if (locationReqHandling.returnStatus_ == 0) {
		locationReqHandling.returnStatus_ = serverReqHandling.returnStatus_;
	}
	if (locationReqHandling.root_.empty()) {
		locationReqHandling.root_ = serverReqHandling.root_;
	}
	if (locationReqHandling.indexFile_.empty()) {
		locationReqHandling.indexFile_ = serverReqHandling.indexFile_;
	}
	if (locationReqHandling.uploadPath_.empty()) {
		locationReqHandling.uploadPath_ = serverReqHandling.uploadPath_;
	}
	if (locationReqHandling.cgiExtension_.empty()) {
		locationReqHandling.cgiExtension_ = serverReqHandling.cgiExtension_;
	}
	if (!locationReqHandling.clientMaxBodySize_.isSet()) {
		locationReqHandling.clientMaxBodySize_ = serverReqHandling.clientMaxBodySize_;
	}
	if (!locationReqHandling.autoIndex_.isSet()) {
		locationReqHandling.autoIndex_ = serverReqHandling.autoIndex_;
	}
}

// request handling에 기본값을 설정하는 함수
void ConfigParser::setDefaultReqHandling(RequestConfig& reqConfig) {
	if (reqConfig.methods_.empty()) {
		// 기본 메서드는 GET, POST, DELETE
		reqConfig.methods_.push_back("GET");
		reqConfig.methods_.push_back("POST");
		reqConfig.methods_.push_back("DELETE");
	}
	if (reqConfig.indexFile_.empty()) {
		// 기본 인덱스 파일은 "index.html"
		reqConfig.indexFile_ = "index.html";
	}
	if (!reqConfig.clientMaxBodySize_.isSet()) {
		// 기본 최대 본문 크기는 1MB
		reqConfig.clientMaxBodySize_ = 1048576;  // 1MB
	}
	if (!reqConfig.autoIndex_.isSet()) {
		// 기본 autoindex 값은 false
		reqConfig.autoIndex_ = false;
	}
}

// 주어진 서버 목록에서 같은 호스트와 포트를 가진 중복 서버가 있는지 확인
bool ConfigParser::isDuplicateServer(const std::vector<ServerConfig>& servers, const ServerConfig& server) {
    for (std::vector<ServerConfig>::const_iterator it = servers.begin(); it != servers.end(); ++it) {
        // 호스트와 포트가 모두 일치하는 서버가 있으면 중복으로 판단
        if (it->host_ == server.host_ && it->port_ == server.port_) {
            return true;
        }
    }
    return false;
}
