#ifndef ERROR_PAGE_RESOLVER_HPP
#define ERROR_PAGE_RESOLVER_HPP

#include "../utils/file_utils.hpp"
#include "GlobalConfig.hpp"
#include <sstream>
#include <map>
#include <string>

#define DEFAULT_ERROR_TEMPLATE "html/error/default_error.html"

// ErrorPageResolver 클래스: 에러 코드에 따른 에러 페이지 파일을 찾고 내용을 반환하는 역할을 합니다.
namespace ErrorPageResolver {
	std::string resolveErrorPage(int errorCode, const RequestConfig& reqConf);
	std::string getErrorDescription(int errorCode);
	std::string getStatusText(int errorCode);
	std::string replaceTemplatePlaceholders(const std::string& templateContent, int errorCode);
};

#endif
