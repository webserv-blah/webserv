#pragma once

#include "ResponseBuilder.hpp"
#include "FileUtilities.hpp"
#include "GlobalConfig.hpp"
#include "Request.hpp"
#include <string>
#include <fstream>
#include <sstream>
#include <vector>

class StaticHandler {
public:
	std::string handleRequest(const Request& req, const ReqHandleConf& currConf);

private:
	// 파일 로딩
	std::string readFile(const std::string &filePath) const;
	// MIME 결정
	std::string determineContentType(const std::string &filePath) const;
};
