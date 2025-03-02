#include "ErrorPageResolver.hpp"
#include "../include/commonEnums.hpp"
#include <algorithm>
#include "file_utils.hpp"

namespace ErrorPageResolver {
	// 상태 코드에 따른 텍스트 설명을 반환하는 함수
	std::string getStatusText(int errorCode) {
		switch (errorCode) {
			case OK: return "OK";
			case MOVED_PERMANENTLY: return "Moved Permanently";
			case FOUND: return "Found";
			case BAD_REQUEST: return "Bad Request";
			case FORBIDDEN: return "Forbidden";
			case NOT_FOUND: return "Not Found";
			case METHOD_NOT_ALLOWED: return "Method Not Allowed";
			case REQUEST_TIMEOUT: return "Request Timeout";
			case CONTENT_TOO_LARGE: return "Payload Too Large";
			case INTERNAL_SERVER_ERROR: return "Internal Server Error";
			case NOT_IMPLEMENTED: return "Not Implemented";
			case SERVICE_UNAVAILABLE: return "Service Unavailable";
			default: return "Unknown Error";
		}
	}

	// 에러 코드에 따른 상세 설명 반환 함수
	std::string getErrorDescription(int errorCode) {
		switch (errorCode) {
			case BAD_REQUEST:
				return "The request could not be understood by the server due to malformed syntax.";
			case FORBIDDEN:
				return "You don't have permission to access this resource.";
			case NOT_FOUND:
				return "The requested resource could not be found on this server.";
			case METHOD_NOT_ALLOWED:
				return "The method specified in the request is not allowed for the resource.";
			case REQUEST_TIMEOUT:
				return "The server timed out waiting for the request.";
			case CONTENT_TOO_LARGE:
				return "The request entity is larger than the server is willing or able to process.";
			case INTERNAL_SERVER_ERROR:
				return "The server encountered an unexpected condition that prevented it from fulfilling the request.";
			case NOT_IMPLEMENTED:
				return "The server does not support the functionality required to fulfill the request.";
			case SERVICE_UNAVAILABLE:
				return "The server is currently unable to handle the request due to temporary overloading or maintenance.";
			default:
				return "An error occurred while processing your request.";
		}
	}

	// 템플릿 파일의 플레이스홀더를 실제 에러 정보로 대체하는 함수
	std::string replaceTemplatePlaceholders(const std::string& templateContent, int errorCode) {
		std::string result = templateContent;
		std::string statusText = getStatusText(errorCode);
		std::string errorDescription = getErrorDescription(errorCode);
		std::string codeStr;
		
		// 에러 코드를 문자열로 변환
		std::ostringstream oss;
		oss << errorCode;
		codeStr = oss.str();
		
		// 플레이스홀더 치환
		size_t pos;
		
		// {{ERROR_CODE}} 대체
		while ((pos = result.find("{{ERROR_CODE}}")) != std::string::npos) {
			result.replace(pos, 14, codeStr);
		}
		
		// {{ERROR_TEXT}} 대체
		while ((pos = result.find("{{ERROR_TEXT}}")) != std::string::npos) {
			result.replace(pos, 14, statusText);
		}
		
		// {{ERROR_DESCRIPTION}} 대체
		while ((pos = result.find("{{ERROR_DESCRIPTION}}")) != std::string::npos) {
			result.replace(pos, 21, errorDescription);
		}
		
		return result;
	}

	// 주어진 에러 코드에 해당하는 에러 페이지의 내용을 반환하는 함수
	std::string resolveErrorPage(int errorCode, const RequestConfig& reqConf) {
		const std::map<int, std::string>& errorPages = reqConf.errorPages_;
		std::string bodyContent; // 에러 페이지의 내용(본문)을 저장할 변수

		// 사용자 정의 에러 페이지를 먼저 찾아봅니다.
		std::map<int, std::string>::const_iterator it = errorPages.find(errorCode);
		if (it != errorPages.end()) {
			const std::string& customPath = FileUtilities::joinPaths(reqConf.root_, it->second);
			bodyContent = FileUtilities::readFile(customPath);
		}

		// 사용자 정의 에러 페이지가 없거나 내용이 비어있다면 기본 템플릿을 사용합니다.
		if (bodyContent.empty()) {
			// 기본 에러 페이지 템플릿 로드
			std::string templateContent = FileUtilities::readFile(DEFAULT_ERROR_TEMPLATE);
			
			// 템플릿이 로드되었으면 플레이스홀더 교체
			if (!templateContent.empty()) {
				bodyContent = replaceTemplatePlaceholders(templateContent, errorCode);
			} 
			// 템플릿을 찾지 못했다면 간단한 HTML 에러 페이지 생성
			else {
				std::ostringstream fallbackHtml;
				fallbackHtml << "<!DOCTYPE html><html><head><title>Error " << errorCode 
							<< "</title></head><body><h1>Error " << errorCode 
							<< " - " << getStatusText(errorCode) 
							<< "</h1><p>" << getErrorDescription(errorCode) 
							<< "</p></body></html>";
				bodyContent = fallbackHtml.str();
			}
		}

		return bodyContent;
	}
}


