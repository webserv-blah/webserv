#ifndef CONFIG_PARSER_HPP
#define CONFIG_PARSER_HPP

#include "../GlobalConfig/GlobalConfig.hpp"
#include "../utils/utils.hpp"

#include <string>			// std::string
#include <vector>			// std::vector
#include <map>				// std::map
#include <fstream>			// std::ifstream
#include <stdexcept>		// std::runtime_error
#include <cctype>			// std::isdigit, std::isxdigit, std::isspace, std::tolower
#include <iterator>			// std::iterator
#include <cstdlib>			// strtoul

class GlobalConfig;
class ServerConfig;
class LocationConfig;
class RequestConfig;

// GlobalConfig::initGlobalConfig() 에서 객체를 초기화할때 사용됩니다.
// 설정 파일 파서
class ConfigParser {
public:
	static void	parse(GlobalConfig& globalConfig, const char* path);

private:
	static void	parseServerBlock(std::ifstream& configFile, ServerConfig& serverBlock);
	static void	parseLocationBlock(std::ifstream& configFile, LocationConfig& locationBlock);
	static void	parseRequestConfig(std::ifstream& configFile, RequestConfig& reqConfig, const std::string& token);

	// LocationBlock이 ServerBlock의 값을 상속받은 후 오버라이드
	static void	getEffectiveReqConfig(const RequestConfig& serverReqConfig, \
										RequestConfig& locationReqConfig);
	// 설정값이 없을 경우 기본값으로 초기화
	static void	setDefaultReqConfig(RequestConfig& ReqConfig);
	// 주어진 서버 목록에서 같은 호스트와 포트를 가진 중복 서버가 있는지 확인
	static bool isDuplicateServer(const std::vector<ServerConfig>& servers, const ServerConfig& server);

	// ServerBlock 파서
	static void	parseHostPort(std::ifstream& configFile, std::string& host, unsigned int& port);
	static void	parseServerNames(std::ifstream& configFile, std::vector<std::string>& serverNames);
	
	// LocationBlock 파서
	static void	parseLocationPath(std::ifstream& configFile, std::string& path);
	static void	parseMethods(std::ifstream& configFile, std::vector<std::string>& methods);

	// RequestConfig 파서
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

#endif
