#include "ConfigParser.hpp"

void ConfigParser::parse(GlobalConfig& globalConfig, const std::string& path) {
    std::ifstream configFile(path.c_str());
    if (!configFile.is_open()) {
        throw std::runtime_error("Failed to open config file: " + path);
    }

    std::string token;
    while (true) {
        token = getNextToken(configFile);
        if (token.empty())
            break;
        if (token == "server") {
            globalConfig.servers.push_back(ServerConfig());
            parseServerBlock(configFile, globalConfig.servers.back());
        } else {
            throw std::runtime_error("Unexpected token in config file: " + token);
        }
    }

    // Get effective request handling for each location block.
    for (std::vector<ServerConfig>::iterator server = globalConfig.servers.begin(); server != globalConfig.servers.end(); ++server) {
        for (std::vector<LocationConfig>::iterator location = server->locations.begin(); location != server->locations.end(); ++location) {
            getEffectiveReqHandling(server->reqHandling, location->reqHandling);
        }
    }
}

void ConfigParser::parseServerBlock(std::ifstream& configFile, ServerConfig& serverBlock) {
    // Expect the opening brace for the server block.
    std::string token = getNextToken(configFile);
    if (token != "{") {
        throw std::runtime_error("Expected '{' after server directive, got: " + token);
    }

    // Process tokens until we hit the closing brace.
    while (true) {
        token = getNextToken(configFile);
        if (token.empty()) {
            throw std::runtime_error("Unexpected end of file in server block");
        }
        // End of server block.
        if (token == "}") {
            break;
        }

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
            parseReqHandleConf(configFile, serverBlock.reqHandling, token);
        }

        token = getNextToken(configFile);
        if (token != ";") {
            throw std::runtime_error("Expected ';' after a simple directive");
        }
    }
}

void ConfigParser::parseLocationBlock(std::ifstream& configFile, LocationConfig& locationBlock) {
    // Retrieve the path token.
    parsePath(configFile, locationBlock.path);

    // Expect the opening brace for the location block.
    std::string token = getNextToken(configFile);
    if (token != "{") {
        throw std::runtime_error("Expected '{' after location directive");
    }

    // Process tokens until we hit the closing brace.
    while (true) {
        token = getNextToken(configFile);
        if (token.empty()) {
            throw std::runtime_error("Unexpected end of file in location block");
        }
        // End of location block.
        if (token == "}") {
            break;
        }

        if (token == "methods") {
            parseMethods(configFile, locationBlock.reqHandling.methods);
        }
        else {
            parseReqHandleConf(configFile, locationBlock.reqHandling, token);
        }

        token = getNextToken(configFile);
        if (token != ";") {
            throw std::runtime_error("Expected ';' after a simple directive");
        }
    }
}

void ConfigParser::parseReqHandleConf(std::ifstream& configFile, ReqHandleConf& reqHandling, const std::string& token) {
    if (token == "error_page") {
        parseErrorPage(configFile, reqHandling.errorPages);
    }
    else if (token == "return") {
        parseReturn(configFile, reqHandling.returnUrl, reqHandling.returnStatus);
    }
    else if (token == "root") {
        parseRoot(configFile, reqHandling.root);
    }
    else if (token == "index") {
        parseIndexFile(configFile, reqHandling.indexFile);
    }
    else if (token == "upload_path") {
        parseUploadPath(configFile, reqHandling.uploadPath);
    }
    else if (token == "cgi_extension") {
        parseCgiExtension(configFile, reqHandling.cgiExtension);
    }
    else if (token == "client_max_body_size") {
        parseClientMaxBodySize(configFile, reqHandling.clientMaxBodySize);
    }
    else if (token == "autoindex") {
        parseAutoIndex(configFile, reqHandling.autoIndex);
    }
    else {
        throw std::runtime_error("Unexpected token: " + token);
    }
}

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
        locationReqHandling.clientMaxBodySize.setValue(serverReqHandling.clientMaxBodySize.value());
    }
    if (!locationReqHandling.autoIndex.isSet()) {
        locationReqHandling.autoIndex.setValue(serverReqHandling.autoIndex.value());
    }
}