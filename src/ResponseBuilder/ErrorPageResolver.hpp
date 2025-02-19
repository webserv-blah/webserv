#pragma once

#include "file_utils.hpp"
#include <sstream>
#include <map>
#include <string>

#define DEFAULT_ERROR_DIRECTORY "../error/"

// ErrorPageResolver 클래스: 에러 코드에 따른 에러 페이지 파일을 찾고 내용을 반환하는 역할을 합니다.
namespace ErrorPageResolver {
	std::string resolveErrorPage(int errorCode, const std::map<int, std::string>& errorPages);
	std::string getReasonPhrase(int errorCode);
};
