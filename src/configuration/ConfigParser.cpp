#include "ConfigParser.hpp"

// globalConfig와 파일 경로를 받아 설정 파일을 파싱하는 함수
void ConfigParser::parse(GlobalConfig& globalConfig, const std::string& path) {
    // 파일 경로로부터 입력 스트림을 생성함
    std::ifstream configFile(path.c_str());
    if (!configFile.is_open()) {
        // 파일을 열지 못하면 예외 발생
        throw std::runtime_error("Failed to open config file: " + path);
    }

    std::string token;
    // 파일의 끝까지 토큰을 읽음
    while (true) {
        token = getNextToken(configFile);
        if (token.empty())
            break;
        // "server" 토큰이 나오면 server 설정 블록을 파싱함
        if (token == "server") {
            globalConfig.servers.push_back(ServerConfig());
            parseServerBlock(configFile, globalConfig.servers.back());
        } else {
            // 예상하지 못한 토큰이면 예외 발생
            throw std::runtime_error("Unexpected token in config file: " + token);
        }
    }

    // 각 location 블록에 대해 server의 request handling을 적용
    for (std::vector<ServerConfig>::iterator server = globalConfig.servers.begin(); server != globalConfig.servers.end(); ++server) {
        for (std::vector<LocationConfig>::iterator location = server->locations.begin(); location != server->locations.end(); ++location) {
            getEffectiveReqHandling(server->reqHandling, location->reqHandling);
        }
    }
    // 각 request handling에 대해 기본값을 설정
    for (std::vector<ServerConfig>::iterator server = globalConfig.servers.begin(); server != globalConfig.servers.end(); ++server) {
        setDefaultReqHandling(server->reqHandling);
        for (std::vector<LocationConfig>::iterator location = server->locations.begin(); location != server->locations.end(); ++location) {
            setDefaultReqHandling(location->reqHandling);
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
            parseHostPort(configFile, serverBlock.host, serverBlock.port);
        }
        else if (token == "server_name") {
            parseServerNames(configFile, serverBlock.serverNames);
        }
        else if (token == "location") {
            serverBlock.locations.push_back(LocationConfig());
            parseLocationBlock(configFile, serverBlock.locations.back());
            continue;
        }
        else {
            // 그 외의 경우 request handling 관련 설정을 파싱함
            parseReqHandleConf(configFile, serverBlock.reqHandling, token);
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
    parsePath(configFile, locationBlock.path);

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
            parseMethods(configFile, locationBlock.reqHandling.methods);
        } else {
            // request handling 관련 설정을 파싱함
            parseReqHandleConf(configFile, locationBlock.reqHandling, token);
        }

        // 각 지시문이 끝난 후 ';'가 있어야 함
        token = getNextToken(configFile);
        if (token != ";") {
            throw std::runtime_error("Expected ';' after a simple directive");
        }
    }
}

// request handling 설정을 파싱하는 함수
void ConfigParser::parseReqHandleConf(std::ifstream& configFile, ReqHandleConf& reqHandling, const std::string& token) {
    if (token == "error_page") {
        parseErrorPage(configFile, reqHandling.errorPages);
    } else if (token == "return") {
        parseReturn(configFile, reqHandling.returnUrl, reqHandling.returnStatus);
    } else if (token == "root") {
        parseRoot(configFile, reqHandling.root);
    } else if (token == "index") {
        parseIndexFile(configFile, reqHandling.indexFile);
    } else if (token == "upload_path") {
        parseUploadPath(configFile, reqHandling.uploadPath);
    } else if (token == "cgi_extension") {
        parseCgiExtension(configFile, reqHandling.cgiExtension);
    } else if (token == "client_max_body_size") {
        parseClientMaxBodySize(configFile, reqHandling.clientMaxBodySize);
    } else if (token == "autoindex") {
        parseAutoIndex(configFile, reqHandling.autoIndex);
    } else {
        // 예상하지 못한 토큰이면 예외 발생
        throw std::runtime_error("Unexpected token: " + token);
    }
}

// 서버 설정의 request handling을 location 설정에 적용하는 함수
void ConfigParser::getEffectiveReqHandling(const ReqHandleConf& serverReqHandling, ReqHandleConf& locationReqHandling) {
    if (locationReqHandling.methods.empty()) {
        locationReqHandling.methods = serverReqHandling.methods;
    }
    if (locationReqHandling.errorPages.empty()) {
        locationReqHandling.errorPages = serverReqHandling.errorPages;
    }
    if (locationReqHandling.returnUrl.empty()) {
        locationReqHandling.returnUrl = serverReqHandling.returnUrl;
    }
    if (locationReqHandling.returnStatus == 0) {
        locationReqHandling.returnStatus = serverReqHandling.returnStatus;
    }
    if (locationReqHandling.root.empty()) {
        locationReqHandling.root = serverReqHandling.root;
    }
    if (locationReqHandling.indexFile.empty()) {
        locationReqHandling.indexFile = serverReqHandling.indexFile;
    }
    if (locationReqHandling.uploadPath.empty()) {
        locationReqHandling.uploadPath = serverReqHandling.uploadPath;
    }
    if (locationReqHandling.cgiExtension.empty()) {
        locationReqHandling.cgiExtension = serverReqHandling.cgiExtension;
    }
    if (!locationReqHandling.clientMaxBodySize.isSet()) {
        locationReqHandling.clientMaxBodySize = serverReqHandling.clientMaxBodySize;
    }
    if (!locationReqHandling.autoIndex.isSet()) {
        locationReqHandling.autoIndex = serverReqHandling.autoIndex;
    }
}

// request handling에 기본값을 설정하는 함수
void ConfigParser::setDefaultReqHandling(ReqHandleConf& reqHandling) {
    if (reqHandling.methods.empty()) {
        // 기본 메서드는 GET, POST, DELETE
        reqHandling.methods.push_back("GET");
        reqHandling.methods.push_back("POST");
        reqHandling.methods.push_back("DELETE");
    }
    if (reqHandling.indexFile.empty()) {
        // 기본 인덱스 파일은 "index.html"
        reqHandling.indexFile = "index.html";
    }
    if (!reqHandling.clientMaxBodySize.isSet()) {
        // 기본 최대 본문 크기는 1MB
        reqHandling.clientMaxBodySize = 1048576;  // 1MB
    }
    if (!reqHandling.autoIndex.isSet()) {
        // 기본 autoindex 값은 false
        reqHandling.autoIndex = false;
    }
}
