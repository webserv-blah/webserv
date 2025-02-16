#pragma once

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

// 설정 파일 파서
class ConfigParser {
public:
	static void	parse(GlobalConfig& globalConfig, const std::string& path);

private:
	static void	parseServerBlock(std::ifstream& configFile, ServerConfig& serverBlock);
	static void	parseLocationBlock(std::ifstream& configFile, LocationConfig& locationBlock);
	static void	parseReqHandleConf(std::ifstream& configFile, ReqHandleConf& reqHandling, const std::string& token);

	// LocationBlock이 ServerBlock의 값을 상속받은 후 오버라이드
	static void	getEffectiveReqHandling(const ReqHandleConf& serverReqHandling, \
										ReqHandleConf& locationReqHandling);
	// 설정값이 없을 경우 기본값으로 초기화
	static void	setDefaultReqHandling(ReqHandleConf& ReqHandling);

	// ServerBlock 파서
	static void	parseHostPort(std::ifstream& configFile, std::string& host, unsigned int& port);
	static void	parseServerNames(std::ifstream& configFile, std::vector<std::string>& serverNames);
	
	// LocationBlock 파서
	static void	parsePath(std::ifstream& configFile, std::string& path);
	static void	parseMethods(std::ifstream& configFile, std::vector<std::string>& methods);

	// ReqHandleConf 파서
	static void	parseErrorPage(std::ifstream& configFile, std::map<int, std::string>& errorPages);
	static void	parseReturn(std::ifstream& configFile, std::string& returnUrl, int& returnStatus);
	static void	parseRoot(std::ifstream& configFile, std::string& root);
	static void	parseIndexFile(std::ifstream& configFile, std::string& indexFile);
	static void	parseUploadPath(std::ifstream& configFile, std::string& uploadPath);
	static void	parseCgiExtension(std::ifstream& configFile, std::string& cgiExtension);
	static void	parseClientMaxBodySize(std::ifstream& configFile, Optional<size_t>& clientMaxBody);
	static void	parseAutoIndex(std::ifstream& configFile, Optional<bool>& autoIndex);

	// 다음 토큰을 하나 읽어오는 함수
	static std::string	getNextToken(std::ifstream& configFile);
	// 다음 토큰을 미리 확인
	static std::string	peekNextToken(std::ifstream& configFile);
};
