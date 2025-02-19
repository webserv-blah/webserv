#include "ErrorPageResolver.hpp" // 헤더 파일 포함

namespace ErrorPageResolver {
	// 주어진 에러 코드에 해당하는 에러 페이지의 내용을 반환하는 함수
	std::string resolveErrorPage(int errorCode, const std::map<int, std::string>& errorPages) {
		std::string bodyContent; // 에러 페이지의 내용(본문)을 저장할 변수

		// errorPages_ 맵에서 해당 에러 코드에 맞는 사용자 정의 에러 페이지 경로를 찾습니다.
		std::map<int, std::string>::const_iterator it = errorPages.find(errorCode);
		if (it != errorPages.end()) {
			const std::string& customPath = it->second; // 사용자 정의 에러 페이지 파일 경로
			bodyContent = FileUtilities::readFile(customPath); // 해당 파일을 읽어 내용 저장
		}

		// 사용자 정의 에러 페이지를 찾지 못했거나 파일 내용이 비어있다면 기본 에러 페이지를 사용합니다.
		if (bodyContent.empty()) {
			std::ostringstream defaultPath;
			// 기본 에러 페이지 경로 생성: 기본 디렉토리 + "/error" + 에러코드 + ".html"
			defaultPath << DEFAULT_ERROR_DIRECTORY << "/error" << errorCode << ".html"; // 에러페이지 파일 이름 컨벤션 필요!
			bodyContent = FileUtilities::readFile(defaultPath.str()); // 기본 경로의 파일을 읽어 내용 저장
		}

		// 파일을 읽었는데도 내용이 없다면 빈 문자열로 처리
		if (bodyContent.empty()) {
			bodyContent = "";
		}

		return bodyContent; // 최종적으로 에러 페이지의 내용을 반환
	}
}


