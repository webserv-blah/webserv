#include "ConfigParser.hpp"

// LocationBlock parser functions
void ConfigParser::parsePath(std::ifstream& configFile, std::string& path) {
    std::string nextToken = getNextToken(configFile);
    if (nextToken.empty()) {
        throw std::runtime_error("Unexpected end of file in location directive");
    } else if (nextToken == ";") {
        throw std::runtime_error("Expected path for location directive");
    }
    path = nextToken;
}

void ConfigParser::parseMethods(std::ifstream& configFile, std::vector<std::string>& methods) {
    std::string nextToken;
    while (true) {
        nextToken = getNextToken(configFile);
        if (nextToken.empty()) {
            throw std::runtime_error("Unexpected end of file in methods directive");
        } else if (nextToken == ";") {
            break;
        } else {
            methods.push_back(nextToken);
        }
    }
}

// parse ReqHandleConf
void ConfigParser::parseErrorPage(std::ifstream& configFile, std::map<int, std::string> &errorPages) {
    std::vector<int> errorCodes;
    std::string      nextToken;

    while (true) {
        nextToken = getNextToken(configFile);
        if (nextToken.empty()) {
            throw std::runtime_error("Unexpected end of file in error_page directive");
        } else if (utils::all_of(nextToken.begin(), nextToken.end(), ::isdigit)) {
            if (nextToken.size() == 3) {
                int errorCode = std::stoi(nextToken);
                errorCodes.push_back(errorCode);
            } else {
                throw std::runtime_error("Invalid error code for error_page directive");
            }
        } else {
            break;
        }
    }

    if (nextToken == ";") {
        throw std::runtime_error("Expected error page URL for error_page directive");
    } else if (errorCodes.empty()) {
        throw std::runtime_error("Expected at least one error code for error_page directive");
    }

    std::string errorPageUrl = nextToken;
    for (size_t i = 0; i < errorCodes.size(); ++i) {
        errorPages[errorCodes[i]] = errorPageUrl;
    }
}

void ConfigParser::parseReturn(std::ifstream& configFile, std::string& returnUrl, int& returnStatus) {
    // Retrieve the status code token
    std::string nextToken = getNextToken(configFile);
    if (nextToken.empty()) {
        throw std::runtime_error("Unexpected end of file in return directive");
    }
    
    // Ensure token is numeric and valid (3 digits)
    if (utils::all_of(nextToken.begin(), nextToken.end(), ::isdigit)) {
        if (nextToken.size() == 3) {
            returnStatus = std::stoi(nextToken);
        } else {
            throw std::runtime_error("Invalid HTTP status code for return directive");
        }
    } else {
        throw std::runtime_error("Expected HTTP status code for return directive");
    }

    // Retrieve the URL token
    nextToken = getNextToken(configFile);
    if (nextToken.empty()) {
        throw std::runtime_error("Unexpected end of file in return directive");
    } else if (nextToken == ";") {
        throw std::runtime_error("Expected URL for return directive");
    }
    returnUrl = nextToken;
}

void ConfigParser::parseRoot(std::ifstream& configFile, std::string& root) {
    std::string nextToken = getNextToken(configFile);
    if (nextToken.empty()) {
        throw std::runtime_error("Unexpected end of file in root directive");
    } else if (nextToken == ";") {
        throw std::runtime_error("Expected root path for root directive");
    }
    root = nextToken;
}

void ConfigParser::parseIndexFile(std::ifstream& configFile, std::string& indexFile) {
    std::string nextToken = getNextToken(configFile);
    if (nextToken.empty()) {
        throw std::runtime_error("Unexpected end of file in index directive");
    } else if (nextToken == ";") {
        throw std::runtime_error("Expected index file name for index directive");
    }
    indexFile = nextToken;
}

void ConfigParser::parseUploadPath(std::ifstream& configFile, std::string& uploadPath) {
    std::string nextToken = getNextToken(configFile);
    if (nextToken.empty()) {
        throw std::runtime_error("Unexpected end of file in upload_path directive");
    } else if (nextToken == ";") {
        throw std::runtime_error("Expected upload path for upload_path directive");
    }
    uploadPath = nextToken;
}

void ConfigParser::parseCgiExtension(std::ifstream& configFile, std::string& cgiExtension) {
    std::string nextToken = getNextToken(configFile);
    if (nextToken.empty()) {
        throw std::runtime_error("Unexpected end of file in cgi_extension directive");
    } else if (nextToken == ";") {
        throw std::runtime_error("Expected CGI extension for cgi_extension directive");
    } else if (nextToken != ".php") {
        throw std::runtime_error("Unsupported CGI extension");
    }
    cgiExtension = nextToken;
}

void ConfigParser::parseClientMaxBodySize(std::ifstream& configFile, Optional<size_t>& clientMaxBody) {
    std::string nextToken = getNextToken(configFile);
    if (nextToken.empty()) {
        throw std::runtime_error("Unexpected end of file in client_max_body_size directive");
    } else if (utils::all_of(nextToken.begin(), nextToken.end(), ::isdigit)) {
        clientMaxBody = std::stoul(nextToken);
    } else {
        throw std::runtime_error("Expected numeric value for client_max_body_size directive");
    }
}

void ConfigParser::parseAutoIndex(std::ifstream& configFile, Optional<bool>& autoIndex) {
    std::string nextToken = getNextToken(configFile);
    if (nextToken.empty()) {
        throw std::runtime_error("Unexpected end of file in autoindex directive");
    } else if (nextToken == "on") {
        autoIndex.setValue(true);
    } else if (nextToken == "off") {
        autoIndex.setValue(false);
    } else {
        throw std::runtime_error("Expected 'on' or 'off' for autoindex directive");
    }
}
