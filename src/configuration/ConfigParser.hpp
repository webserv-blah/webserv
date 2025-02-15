#ifndef CONFIGPARSER_HPP
# define CONFIGPARSER_HPP

#include "GlobalConfig.hpp"
#include "utilities.hpp"

#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <stdexcept>
#include <cctype>
#include <stdexcept>
#include <algorithm>
#include <iterator>

class ConfigParser {
public:
    static void parse(GlobalConfig& globalConfig, const std::string& path);

private:
    static void parseServerBlock(std::ifstream& configFile, ServerConfig& serverBlock);
    static void parseLocationBlock(std::ifstream& configFile, LocationConfig& locationBlock);
    static void parseReqHandleConf(std::ifstream& configFile, ReqHandleConf& reqHandling, const std::string& token);
    static void getEffectiveReqHandling(const ReqHandleConf& serverReqHandling, \
                                        ReqHandleConf& locationReqHandling);
    // parse ServerBlock
    static void parseHostPort(std::ifstream& configFile, std::string& host, unsigned int& port);
    static void parseServerNames(std::ifstream& configFile, std::vector<std::string>& serverNames);
    
    // parse LocationBlock
    static void parsePath(std::ifstream& configFile, std::string& path);
    static void parseMethods(std::ifstream& configFile, std::vector<std::string>& methods);

    // parse ReqHandleConf
    static void parseErrorPage(std::ifstream& configFile, std::map<int, std::string>& errorPages);
    static void parseReturn(std::ifstream& configFile, std::string& returnUrl, int& returnStatus);
    static void parseRoot(std::ifstream& configFile, std::string& root);
    static void parseIndexFile(std::ifstream& configFile, std::string& indexFile);
    static void parseUploadPath(std::ifstream& configFile, std::string& uploadPath);
    static void parseCgiExtension(std::ifstream& configFile, std::string& cgiExtension);
    static void parseClientMaxBodySize(std::ifstream& configFile, Optional<size_t>& clientMaxBody);
    static void parseAutoIndex(std::ifstream& configFile, Optional<bool>& autoIndex);

    static std::string getNextToken(std::ifstream& configFile);
    // static std::string peekNextToken(std::ifstream& configFile);
};

#endif